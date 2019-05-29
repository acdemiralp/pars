import parse

from bokeh.plotting import figure, output_file, show
from bokeh.models   import Span

benchmark = parse.parse_benchmark_scaling_multi([
    "../../build/msvc142/pars_benchmark/test.json.nodes.32.threads.24.csv",
    "../../build/msvc142/pars_benchmark/test.json.nodes.48.threads.24.csv",
    "../../build/msvc142/pars_benchmark/test.json.nodes.62.threads.24.csv"])
print(benchmark)

keys            = []
data_loader     = []
seed_generator  = []
particle_tracer = []
index_generator = []
color_generator = []
ray_tracer      = []

for key in benchmark:
    keys           .append(str(key))
    data_loader    .append(benchmark[key]["data_loader"    ])
    seed_generator .append(benchmark[key]["seed_generator" ])
    particle_tracer.append(benchmark[key]["particle_tracer"])
    index_generator.append(benchmark[key]["index_generator"])
    color_generator.append(benchmark[key]["color_generator"])
    ray_tracer     .append(benchmark[key]["ray_tracer"     ])

figure = figure(title="PRS Strong Scaling", x_axis_label='Processes', y_axis_label='Time', sizing_mode="scale_height", x_range=keys)

figure.line(keys, data_loader    , legend="Data Loader"    , line_width=2, line_color="sandybrown"    )
figure.line(keys, seed_generator , legend="Seed Generator" , line_width=2, line_color="olivedrab"     )
figure.line(keys, particle_tracer, legend="Particle Tracer", line_width=2, line_color="dodgerblue"    )
figure.line(keys, index_generator, legend="Index Generator", line_width=2, line_color="turquoise"     )
figure.line(keys, color_generator, legend="Color Generator", line_width=2, line_color="slateblue"     )
figure.line(keys, ray_tracer     , legend="Ray Tracer"     , line_width=2, line_color="crimson"       )

output_file("scaling.html")
show       (figure)