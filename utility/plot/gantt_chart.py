import parse

from bokeh.plotting import figure, output_file, show
from bokeh.models   import Span

benchmark = parse.parse_benchmark_gantt("../../build/msvc142/pars_benchmark/test.json.nodes.2.threads.6.csv")
print(benchmark)

figure = figure(title="PRS Gantt Chart", x_axis_label='Time', y_axis_label='Rank', sizing_mode="scale_height")
figure.yaxis.major_tick_line_color      = None
figure.yaxis.minor_tick_line_color      = None
figure.yaxis.major_label_text_font_size = '0pt'

offsets = [0] * len(benchmark[0])
for rank_index in range(len(benchmark)):
    for iteration_index in range(len(benchmark[rank_index])):
        offsets[iteration_index] = max(offsets[iteration_index], 
                benchmark[rank_index][iteration_index]["load_balancing"] + 
                benchmark[rank_index][iteration_index]["tracing"] + 
                benchmark[rank_index][iteration_index]["communication"]);        

for rank_index in range(len(benchmark)):
    offset = 0
    for iteration_index in range(len(benchmark[rank_index])):
        load_balancing = benchmark[rank_index][iteration_index]["load_balancing"]
        tracing        = benchmark[rank_index][iteration_index]["tracing"       ]
        communication  = benchmark[rank_index][iteration_index]["communication" ]

        figure.add_layout(Span(location=offset, dimension='height', line_color='black', line_width=1))

        figure.hbar(y=rank_index, height=0.9, left=offset                           , right=offset + load_balancing                          , color="#73ABC5")
        figure.hbar(y=rank_index, height=0.9, left=offset + load_balancing          , right=offset + load_balancing + tracing                , color="#73C5B6")
        figure.hbar(y=rank_index, height=0.9, left=offset + load_balancing + tracing, right=offset + load_balancing + tracing + communication, color="#F4A460")
        offset += offsets[iteration_index]

output_file("gantt.html")
show       (figure)