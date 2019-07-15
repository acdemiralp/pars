import parse

from bokeh.io import export_png
from bokeh.layouts import row
from bokeh.plotting import figure, output_file, show
from bokeh.models   import Span

# Node hard scaling example.
benchmark = parse.parse_benchmark_node_scaling([
    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n8_p48_st16_i1024_lb0.json.csv" ,
    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n16_p48_st16_i1024_lb0.json.csv",
    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n32_p48_st16_i1024_lb0.json.csv",
    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n64_p48_st16_i1024_lb0.json.csv"])

# Node soft scaling example.
#benchmark = parse.parse_benchmark_node_scaling([
#    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n8_p24_st64_i1024_lb1.csv",
#    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n16_p24_st32_i1024_lb1.csv",
#    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n32_p24_st16_i1024_lb1.csv",
#    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n64_p24_st8_i1024_lb1.csv",
#    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n128_p24_st4_i1024_lb1.csv"])

# Processor hard scaling example.
#benchmark = parse.parse_benchmark_processor_scaling([
#    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n32_p4_st16_i1024_lb1.csv",
#    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n32_p8_st16_i1024_lb1.csv",
#    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n32_p16_st16_i1024_lb1.csv",
#    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n32_p24_st16_i1024_lb1.csv"])

# Process soft scaling example.
#benchmark = parse.parse_benchmark_processor_scaling([
#    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n32_p4_st16_i1024_lb1.csv",
#    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n32_p8_st8_i1024_lb1.csv",
#    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n32_p16_st4_i1024_lb1.csv"])

print(benchmark)

keys             = []
data_loader      = []
seed_generator   = []
particle_tracer  = []
index_generator  = []
color_generator  = []
ray_tracer_setup = []
ray_tracer_trace = []
total            = []

for key in benchmark:
    keys            .append(str(key))
    data_loader     .append(benchmark[key]["data_loader"     ])
    seed_generator  .append(benchmark[key]["seed_generator"  ])
    particle_tracer .append(benchmark[key]["particle_tracer" ])
    index_generator .append(benchmark[key]["index_generator" ])
    color_generator .append(benchmark[key]["color_generator" ])
    ray_tracer_setup.append(benchmark[key]["ray_tracer_setup"])
    ray_tracer_trace.append(benchmark[key]["ray_tracer_trace"])
    total           .append(benchmark[key]["data_loader"     ] +
                            benchmark[key]["seed_generator"  ] +
                            benchmark[key]["particle_tracer" ] +
                            benchmark[key]["index_generator" ] +
                            benchmark[key]["color_generator" ] +
                            benchmark[key]["ray_tracer_setup"] + 
                            benchmark[key]["ray_tracer_trace"])

def generate(data, name, color):
    plot = figure(title=name, x_axis_label='Nodes/Processors', y_axis_label='Time', sizing_mode="scale_height", x_range=keys)
    plot.line(keys, data, line_width=2, line_color=color)
    return plot

plot = row(
    generate(data_loader     , "Data Loader"     , "sandybrown"),
    generate(seed_generator  , "Seed Generator"  , "olivedrab" ),
    generate(particle_tracer , "Particle Tracer" , "dodgerblue"),
    generate(index_generator , "Index Generator" , "turquoise" ),
    generate(color_generator , "Color Generator" , "slateblue" ),
    generate(ray_tracer_setup, "Ray Tracer Setup", "crimson"   ),
    generate(ray_tracer_trace, "Ray Tracer Trace", "red"       ),
    generate(total           , "Total"           , "black"     ))

output_file("scaling.html")
export_png (plot, filename="scaling.png")
show       (plot)