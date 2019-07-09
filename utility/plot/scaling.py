import parse

from bokeh.plotting import figure, output_file, show
from bokeh.models   import Span

# Node hard scaling example.
benchmark = parse.parse_benchmark_node_scaling([
    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n8_p24_st16_i1024_lb1.csv",
    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n16_p24_st16_i1024_lb1.csv",
    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n24_p24_st16_i1024_lb1.csv",
    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n32_p24_st16_i1024_lb1.csv",
    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n64_p24_st16_i1024_lb1.csv",
    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n128_p24_st16_i1024_lb1.csv",
    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n256_p24_st16_i1024_lb1.csv"])

# Node soft scaling example.
benchmark = parse.parse_benchmark_node_scaling([
    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n8_p24_st64_i1024_lb1.csv",
    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n16_p24_st32_i1024_lb1.csv",
    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n32_p24_st16_i1024_lb1.csv",
    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n64_p24_st8_i1024_lb1.csv",
    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n128_p24_st4_i1024_lb1.csv"])

# Processor hard scaling example.
benchmark = parse.parse_benchmark_processor_scaling([
    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n32_p4_st16_i1024_lb1.csv",
    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n32_p8_st16_i1024_lb1.csv",
    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n32_p16_st16_i1024_lb1.csv",
    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n32_p24_st16_i1024_lb1.csv"])

# Process soft scaling example.
benchmark = parse.parse_benchmark_processor_scaling([
    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n32_p4_st16_i1024_lb1.csv",
    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n32_p8_st8_i1024_lb1.csv",
    "D:/root/source/cpp/in_progress/pars/benchmarks/benchmark_sc4_n32_p16_st4_i1024_lb1.csv"])

print(benchmark)

keys            = []
data_loader     = []
seed_generator  = []
particle_tracer = []
index_generator = []
color_generator = []
ray_tracer      = []
total           = []

for key in benchmark:
    keys           .append(str(key))
    data_loader    .append(benchmark[key]["data_loader"    ])
    seed_generator .append(benchmark[key]["seed_generator" ])
    particle_tracer.append(benchmark[key]["particle_tracer"])
    index_generator.append(benchmark[key]["index_generator"])
    color_generator.append(benchmark[key]["color_generator"])
    ray_tracer     .append(benchmark[key]["ray_tracer"     ])
    total          .append(benchmark[key]["data_loader"    ] +
                           benchmark[key]["seed_generator" ] +
                           benchmark[key]["particle_tracer"] +
                           benchmark[key]["index_generator"] +
                           benchmark[key]["color_generator"] +
                           benchmark[key]["ray_tracer"     ])

figure = figure(title="PRS Scaling", x_axis_label='Nodes/Processors', y_axis_label='Time', sizing_mode="scale_height", x_range=keys)

figure.line(keys, data_loader    , legend="Data Loader"    , line_width=2, line_color="sandybrown")
figure.line(keys, seed_generator , legend="Seed Generator" , line_width=2, line_color="olivedrab" )
figure.line(keys, particle_tracer, legend="Particle Tracer", line_width=2, line_color="dodgerblue")
figure.line(keys, index_generator, legend="Index Generator", line_width=2, line_color="turquoise" )
figure.line(keys, color_generator, legend="Color Generator", line_width=2, line_color="slateblue" )
figure.line(keys, ray_tracer     , legend="Ray Tracer"     , line_width=2, line_color="crimson"   )
figure.line(keys, total          , legend="Total"          , line_width=2, line_color="black"     )

output_file("scaling.html")
show       (figure)