import parse

from bokeh.plotting import figure, output_file, show
from bokeh.models   import Span

benchmark = parse.parse_benchmark_scaling_multi([
    "../../build/msvc142/benchmark/test.json.nodes.1.threads.6.csv",
    "../../build/msvc142/benchmark/test.json.nodes.2.threads.6.csv",
    "../../build/msvc142/benchmark/test.json.nodes.3.threads.6.csv",
    "../../build/msvc142/benchmark/test.json.nodes.4.threads.6.csv"])
print(benchmark)

keys            = []
partitioner     = []
dataset_loader  = []
particle_tracer = []
color_mapper    = []
raytracer       = []
for key in benchmark:
    keys           .append(str(key))
    partitioner    .append(benchmark[key]["partitioner"    ])
    dataset_loader .append(benchmark[key]["dataset_loader" ])
    particle_tracer.append(benchmark[key]["particle_tracer"])
    color_mapper   .append(benchmark[key]["color_mapper"   ])
    raytracer      .append(benchmark[key]["raytracer"      ])

figure = figure(title="PRS Strong Scaling", x_axis_label='Processes', y_axis_label='Time', sizing_mode="scale_height", x_range=keys)

figure.line(keys, partitioner    , legend="Partitioner"    , line_width=2, line_color="red"   )
figure.line(keys, dataset_loader , legend="Dataset Loader" , line_width=2, line_color="green" )
figure.line(keys, particle_tracer, legend="Particle Tracer", line_width=2, line_color="blue"  )
figure.line(keys, color_mapper   , legend="Color Mapper"   , line_width=2, line_color="cyan"  )
figure.line(keys, raytracer      , legend="Raytracer"      , line_width=2, line_color="orange")

output_file("scaling.html")
show       (figure)