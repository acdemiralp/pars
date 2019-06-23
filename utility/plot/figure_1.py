from bokeh.io import export_png
from bokeh.layouts import row
from bokeh.models import Arrow, ColorBar, ColumnDataSource, LabelSet, LinearColorMapper, VeeHead
from bokeh.plotting import figure, output_file, show
from colour import Color
from PIL import Image
from random import randint
import copy
import numpy

def generate_initial(data):
    maximum    = 255
    colors     = Color("#ffffff").range_to(Color("#aaaaaa"), maximum)
    colors_hex = []
    for color in colors:
        colors_hex.append(color.hex_l)

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
    plot.rect(x="x", y="y", width=1, height=1, source=source, fill_color={'field': 'value', 'transform': mapper}, line_color="#000000", line_alpha=1)

    return plot

def generate(data, incoming_arrow_data, outgoing_arrow_data):
    maximum    = 10
    colors     = Color("white").range_to(Color("red"), maximum)
    colors_hex = []
    for color in colors:
        colors_hex.append(color.hex_l)

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
    labels = LabelSet(x="x", y="y", text="value", text_align="center", text_font="helvetica", text_font_size="16pt", x_offset=0, y_offset=-12, source=source, render_mode='canvas')
    plot.rect(x="x", y="y", width=1, height=1, source=source, fill_color="#ffffff", line_color="#000000", line_alpha=1)
    
    if incoming_arrow_data:
        incoming_arrow_source = ColumnDataSource(incoming_arrow_data)
        incoming_arrows = Arrow(end=VeeHead(fill_color="gray"), source=incoming_arrow_source, x_start='x_start', y_start='y_start', x_end='x_end', y_end='y_end', line_color="gray")
        plot.add_layout(incoming_arrows)

    if outgoing_arrow_data:
        outgoing_arrow_source = ColumnDataSource(outgoing_arrow_data)
        outgoing_arrows = Arrow(end=VeeHead(fill_color="black"), source=outgoing_arrow_source, x_start='x_start', y_start='y_start', x_end='x_end', y_end='y_end', line_color="black")
        plot.add_layout(outgoing_arrows)

    return plot

width   = 3
height  = 3

#data = {"x": [], "y": [], "value": []}
#for x in range(0, width):
#    for y in range(0, height):
#        data["x"    ].append(x)
#        data["y"    ].append(y)
#        data["value"].append(randint(0, maximum))

#image = Image.open('lena.png').convert('LA').resize((width, height))
#data = {"x": [], "y": [], "value": []}
#for x in range(0, width):
#    for y in range(0, height):
#        data["x"    ].append(x)
#        data["y"    ].append(y)
#        data["value"].append(image.getpixel((image.size[0] - 1 - x, image.size[1] - 1 - y))[0])

initial_data = {"x": [], "y": [], "value": [0, 225, 0, 225, 225, 225, 0, 225, 0]}
for x in range(0, 3):
    for y in range(0, 3):
        initial_data["x"    ].append(x)
        initial_data["y"    ].append(y)

data = {"x": [], "y": [], "value": [0, 1, 0, 2, 4, 3, 0, 10, 0]}
for x in range(0, 3):
    for y in range(0, 3):
        data["x"    ].append(x)
        data["y"    ].append(y)

plot = row(generate_initial(initial_data), generate_initial(initial_data))

output_file("figure_1.html")
export_png (plot, filename="figure_1.png")
show       (plot)
