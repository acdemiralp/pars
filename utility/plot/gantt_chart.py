import parse

from bokeh.io import export_png
from bokeh.layouts import column
from bokeh.plotting import figure, output_file, show
from bokeh.models   import Span

def generate(benchmark):
    print(benchmark)
    
    plot = figure(title="", x_axis_label='Time', y_axis_label='Rank', sizing_mode="stretch_width", x_range=(0, 300), plot_width=1920)
    plot.yaxis.major_tick_line_color      = None
    plot.yaxis.minor_tick_line_color      = None
    plot.yaxis.major_label_text_font_size = '0pt'
    plot.toolbar.logo                     = None
    plot.toolbar_location                 = None
    
    offsets = [0] * len(benchmark[0])
    for rank_index in range(len(benchmark)):
        for iteration_index in range(len(benchmark[rank_index])):
            offsets[iteration_index] = max(offsets[iteration_index], 
                    benchmark[rank_index][iteration_index]["load_balancing"] + 
                    benchmark[rank_index][iteration_index]["tracing"       ] + 
                    benchmark[rank_index][iteration_index]["communication" ]);        
    
    for rank_index in range(len(benchmark)):
        offset = 0
        for iteration_index in range(len(benchmark[rank_index])):
            load_balancing = benchmark[rank_index][iteration_index]["load_balancing"]
            tracing        = benchmark[rank_index][iteration_index]["tracing"       ]
            communication  = benchmark[rank_index][iteration_index]["communication" ]
    
            plot.add_layout(Span(location=offset, dimension='height', line_color='black', line_width=1))
    
            plot.hbar(y=rank_index, height=0.9, left=offset                           , right=offset + load_balancing                          , color="#73ABC5")
            plot.hbar(y=rank_index, height=0.9, left=offset + load_balancing          , right=offset + load_balancing + tracing                , color="#73C5B6")
            plot.hbar(y=rank_index, height=0.9, left=offset + load_balancing + tracing, right=offset + load_balancing + tracing + communication, color="#F4A460")
            offset += offsets[iteration_index]

    return plot


plot = column(
    generate(parse.parse_benchmark_gantt("D:/root/source/cpp/in_progress/pars/build/msvc142/pars_benchmark/benchmark_sc2_n64_p4_st32_i1024_lb0.json.csv")), 
    generate(parse.parse_benchmark_gantt("D:/root/source/cpp/in_progress/pars/build/msvc142/pars_benchmark/benchmark_sc2_n64_p4_st32_i1024_lb1.json.csv")))
output_file("gantt.html")
export_png (plot, filename="gantt.png")
show       (plot)