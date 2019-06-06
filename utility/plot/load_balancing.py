from bokeh.io import export_png
from bokeh.layouts import row
from bokeh.models import ColorBar, ColumnDataSource, LabelSet, LinearColorMapper
from bokeh.plotting import figure, output_file, show
from colour import Color
from PIL import Image
from random import randint
import copy
import numpy

def generate(data):
    plot = figure(title="", x_axis_label='', y_axis_label='', sizing_mode="scale_height")
    plot.xgrid.grid_line_color            = None
    plot.ygrid.grid_line_color            = None
    plot.xaxis.major_tick_line_color      = None
    plot.xaxis.minor_tick_line_color      = None
    plot.xaxis.major_label_text_color     = '#ffffff'
    plot.xaxis.major_label_text_font_size = '0px'
    plot.xaxis.axis_line_color            = "#ffffff"
    plot.yaxis.major_tick_line_color      = None
    plot.yaxis.minor_tick_line_color      = None
    plot.yaxis.major_label_text_color     = '#ffffff'
    plot.yaxis.major_label_text_font_size = '0px'
    plot.yaxis.axis_line_color            = "#ffffff"
    plot.toolbar.logo                     = None
    plot.toolbar_location                 = None

    source = ColumnDataSource(data)
    mapper = LinearColorMapper(palette=colors_hex, low=0, high=len(colors_hex))
    #labels = LabelSet(x="x", y="y", text="value", text_align="center", text_font="helvetica", text_font_size="16pt", x_offset=0, y_offset=-12, source=source, render_mode='canvas')
    plot.rect(x="x", y="y", width=1, height=1, source=source, fill_color={'field': 'value', 'transform': mapper}, line_color="#000000", line_alpha=0)
    #plot.add_layout(labels)

    return plot

def load_balance(data, width, height):
    balanced_data    = copy.deepcopy(data)
    west_acceptance  = [0] * width * height
    east_acceptance  = [0] * width * height
    south_acceptance = [0] * width * height
    north_acceptance = [0] * width * height
    west_outgoing    = [0] * width * height
    east_outgoing    = [0] * width * height
    south_outgoing   = [0] * width * height
    north_outgoing   = [0] * width * height
    
    for x in range(0, width):
        for y in range(0, height):
            index            = numpy.ravel_multi_index((x, y), (width, height))
            neighbor_indices = [-1, -1, -1, -1]
            if (x > 0)        : neighbor_indices[0] = numpy.ravel_multi_index((x - 1, y    ), (width, height))
            if (x < width - 1): neighbor_indices[1] = numpy.ravel_multi_index((x + 1, y    ), (width, height))
            if (y > 0)        : neighbor_indices[2] = numpy.ravel_multi_index((x    , y - 1), (width, height))
            if (y < width - 1): neighbor_indices[3] = numpy.ravel_multi_index((x    , y + 1), (width, height))
            
            local_workload = data["value"][index]
            west_workload  = data["value"][neighbor_indices[0]] if neighbor_indices[0] != -1 else -1
            east_workload  = data["value"][neighbor_indices[1]] if neighbor_indices[1] != -1 else -1
            south_workload = data["value"][neighbor_indices[2]] if neighbor_indices[2] != -1 else -1
            north_workload = data["value"][neighbor_indices[3]] if neighbor_indices[3] != -1 else -1
            
            # Take the average of neighbors with workload more than this process.
            local_sum      = local_workload
            local_count    = 1
            if (west_workload  != -1 and west_workload  > local_workload):
                local_sum   += west_workload
                local_count += 1
            if (east_workload  != -1 and east_workload  > local_workload):
                local_sum   += east_workload
                local_count += 1
            if (south_workload != -1 and south_workload > local_workload):
                local_sum   += south_workload
                local_count += 1
            if (north_workload != -1 and north_workload > local_workload):
                local_sum   += north_workload
                local_count += 1
            local_average = int(local_sum / local_count)

            # Take the average of neighbors with workload more than the average until all remaining neighbors are above the average.
            for i in range(0, 4):
                west_contributes  = False
                east_contributes  = False
                south_contributes = False
                north_contributes = False

                local_sum   = local_workload
                local_count = 1

                if (west_workload  != -1 and west_workload  > local_average):
                    local_sum        += west_workload
                    local_count      += 1
                    west_contributes = True
                if (east_workload  != -1 and east_workload  > local_average):
                    local_sum        += east_workload
                    local_count      += 1
                    east_contributes = True
                if (south_workload != -1 and south_workload > local_average):
                    local_sum         += south_workload
                    local_count       += 1
                    south_contributes = True
                if (north_workload != -1 and north_workload > local_average):
                    local_sum         += north_workload
                    local_count       += 1
                    north_contributes = True
                local_average = int(local_sum / local_count)

            # Compute acceptance.
            acceptance_workload        = local_average - local_workload
            acceptance_neighbor_totals = 0
            if (west_contributes  ) : 
                acceptance_neighbor_totals += west_workload
            if (east_contributes  ) : 
                acceptance_neighbor_totals += east_workload
            if (south_contributes ) : 
                acceptance_neighbor_totals += south_workload
            if (north_contributes ) : 
                acceptance_neighbor_totals += north_workload
            
            if (west_contributes  ) : 
                west_acceptance [index] = int(acceptance_workload * west_workload  / acceptance_neighbor_totals)
            if (east_contributes  ) : 
                east_acceptance [index] = int(acceptance_workload * east_workload  / acceptance_neighbor_totals)
            if (south_contributes ) : 
                south_acceptance[index] = int(acceptance_workload * south_workload / acceptance_neighbor_totals)
            if (north_contributes ) : 
                north_acceptance[index] = int(acceptance_workload * north_workload / acceptance_neighbor_totals)
            
    for x in range(0, width):
        for y in range(0, height):
            index            = numpy.ravel_multi_index((x, y), (width, height))
            neighbor_indices = [-1, -1, -1, -1]
            if (x > 0)        : neighbor_indices[0] = numpy.ravel_multi_index((x - 1, y    ), (width, height))
            if (x < width - 1): neighbor_indices[1] = numpy.ravel_multi_index((x + 1, y    ), (width, height))
            if (y > 0)        : neighbor_indices[2] = numpy.ravel_multi_index((x    , y - 1), (width, height))
            if (y < width - 1): neighbor_indices[3] = numpy.ravel_multi_index((x    , y + 1), (width, height))
            
            local_workload = data["value"][index]
            west_workload  = data["value"][neighbor_indices[0]] if neighbor_indices[0] != -1 else -1
            east_workload  = data["value"][neighbor_indices[1]] if neighbor_indices[1] != -1 else -1
            south_workload = data["value"][neighbor_indices[2]] if neighbor_indices[2] != -1 else -1
            north_workload = data["value"][neighbor_indices[3]] if neighbor_indices[3] != -1 else -1
            
            # Take the average of neighbors with workload less than this process.
            local_sum      = local_workload
            local_count    = 1
            if (west_workload  != -1 and west_workload  < local_workload):
                local_sum   += west_workload
                local_count += 1
            if (east_workload  != -1 and east_workload  < local_workload):
                local_sum   += east_workload
                local_count += 1
            if (south_workload != -1 and south_workload < local_workload):
                local_sum   += south_workload
                local_count += 1
            if (north_workload != -1 and north_workload < local_workload):
                local_sum   += north_workload
                local_count += 1
            local_average = int(local_sum / local_count)

            # Take the average of neighbors with workload less than the average until all remaining neighbors are below the average.
            for i in range(0, 4):
                west_contributes  = False
                east_contributes  = False
                south_contributes = False
                north_contributes = False

                local_sum   = local_workload
                local_count = 1

                if (west_workload  != -1 and west_workload  < local_average):
                    local_sum        += west_workload
                    local_count      += 1
                    west_contributes = True
                if (east_workload  != -1 and east_workload  < local_average):
                    local_sum        += east_workload
                    local_count      += 1
                    east_contributes = True
                if (south_workload != -1 and south_workload < local_average):
                    local_sum         += south_workload
                    local_count       += 1
                    south_contributes = True
                if (north_workload != -1 and north_workload < local_average):
                    local_sum         += north_workload
                    local_count       += 1
                    north_contributes = True
                local_average = int(local_sum / local_count)

            # Compute redistribution, based on acceptance.
            if (west_contributes  ) : 
                west_outgoing [index] = int(local_average - west_workload)
            if (east_contributes  ) : 
                east_outgoing [index] = int(local_average - east_workload)
            if (south_contributes ) : 
                south_outgoing[index] = int(local_average - south_workload)
            if (north_contributes ) : 
                north_outgoing[index] = int(local_average - north_workload)
        
    for x in range(0, width):
        for y in range(0, height):
            index            = numpy.ravel_multi_index((x, y), (width, height))
            neighbor_indices = [-1, -1, -1, -1]
            if (x > 0)        : neighbor_indices[0] = numpy.ravel_multi_index((x - 1, y    ), (width, height))
            if (x < width - 1): neighbor_indices[1] = numpy.ravel_multi_index((x + 1, y    ), (width, height))
            if (y > 0)        : neighbor_indices[2] = numpy.ravel_multi_index((x    , y - 1), (width, height))
            if (y < width - 1): neighbor_indices[3] = numpy.ravel_multi_index((x    , y + 1), (width, height))

            west_outgoing_bound  = min(west_outgoing [index], east_acceptance [neighbor_indices[0]])
            east_outgoing_bound  = min(east_outgoing [index], west_acceptance [neighbor_indices[1]])
            south_outgoing_bound = min(south_outgoing[index], north_acceptance[neighbor_indices[2]])
            north_outgoing_bound = min(north_outgoing[index], south_acceptance[neighbor_indices[3]])

            balanced_data["value"][neighbor_indices[0]] +=  west_outgoing_bound
            balanced_data["value"][neighbor_indices[1]] +=  east_outgoing_bound
            balanced_data["value"][neighbor_indices[2]] +=  south_outgoing_bound
            balanced_data["value"][neighbor_indices[3]] +=  north_outgoing_bound
            balanced_data["value"][index]               -= (west_outgoing_bound  + 
                                                            east_outgoing_bound  + 
                                                            south_outgoing_bound + 
                                                            north_outgoing_bound )
            
    return balanced_data

width   = 100
height  = 100
maximum = 255

data = {"x": [], "y": [], "value": []}
for x in range(0, width):
    for y in range(0, height):
        data["x"    ].append(x)
        data["y"    ].append(y)
        data["value"].append(randint(0, maximum))

#image = Image.open('gauss.jpg').convert('LA').resize((width, height))
#data = {"x": [], "y": [], "value": []}
#for x in range(0, width):
#    for y in range(0, height):
#        data["x"    ].append(x)
#        data["y"    ].append(y)
#        data["value"].append(image.getpixel((image.size[0] - 1 - x, image.size[1] - 1 - y))[0])

#data   = {"x": [], "y": [], "value": [1200, 600, 300, 700, 400, 100, 1800, 1500, 900, 1100, 500, 300, 1000, 100, 600, 700, 1300, 200, 1400, 800, 200, 800, 400, 500, 1600]}
#for x in range(0, 5):
#    for y in range(0, 5):
#        data["x"    ].append(x)
#        data["y"    ].append(y)

colors     = Color("white").range_to(Color("red"), maximum)
colors_hex = []
for color in colors:
    colors_hex.append(color.hex_l)

plot = row(
    generate(data), 
    generate(load_balance(data, width, height)), 
    generate(load_balance(load_balance(data, width, height), width, height)), 
    generate(load_balance(load_balance(load_balance(data, width, height), width, height), width, height)), 
    generate(load_balance(load_balance(load_balance(load_balance(data, width, height), width, height), width, height), width, height)),
    generate(load_balance(load_balance(load_balance(load_balance(load_balance(data, width, height), width, height), width, height), width, height), width, height)))
output_file("heatmap.html")
export_png (plot, filename="heatmap.png")
show       (plot)
