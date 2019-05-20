import csv
import os
import re

def parse_benchmark              (filepath):
    benchmark = []
    with open(filepath, mode='r') as csv_file:
        csv_reader = csv.DictReader(csv_file)
        for row in csv_reader:
            benchmark.append(row)
    return benchmark

# Format: [[{"tracing": 123.4, "communication": 567.8}, ...]]
def parse_benchmark_gantt        (filepath):
    raw_benchmark = parse_benchmark(filepath)

    benchmark = []
    for row in raw_benchmark:
        rank  = int  (row["rank"])
        name  =       row["name"]
        value = float(row["iteration 0"])

        if rank >= len(benchmark):
            benchmark.extend([[]] * (rank + 1 - len(benchmark)))
        
        if (re.match("3\..*\.6", name) or
            re.match("3\..*\.7", name) or
            re.match("3\..*\.8", name) or
            re.match("3\..*\.9", name)):
            iteration = int(name.split(".")[1])
            if iteration >= len(benchmark[rank]):
                benchmark[rank].extend([{"tracing": 0.0, "communication": 0.0}] * (iteration + 1 - len(benchmark[rank])))
            benchmark[rank][iteration]["tracing"]       += value

        if (re.match("3\..*\.10", name) or
            re.match("3\..*\.11", name)):
            iteration = int(name.split(".")[1])
            if iteration >= len(benchmark[rank]):
                benchmark[rank].extend([{"tracing": 0.0, "communication": 0.0}] * (iteration + 1 - len(benchmark[rank])))
            benchmark[rank][iteration]["communication"] += value
            
    return benchmark

# Format:       {"partitioner": 123.4, "dataset_loader": 567.8, "particle_tracer": 123.4, "color_mapper": 567.8, "raytracer": 123.4}
def parse_benchmark_scaling      (filepath):
    raw_benchmark = parse_benchmark(filepath)

    benchmark = []
    for row in raw_benchmark:
        rank  = int  (row["rank"])
        name  =       row["name"]
        value = float(row["iteration 0"])

        if rank >= len(benchmark):
            benchmark.extend([{"partitioner": 0.0, "dataset_loader": 0.0, "particle_tracer": 0.0, "color_mapper": 0.0, "raytracer": 0.0}] * (rank + 1 - len(benchmark)))
        
        if re.match("1\..*", name):
            benchmark[rank]["partitioner"    ] += value          
        if re.match("2\..*", name):
            benchmark[rank]["dataset_loader" ] += value
        if re.match("3\..*", name):
            benchmark[rank]["particle_tracer"] += value
        if re.match("4\..*", name):
            benchmark[rank]["color_mapper"   ] += value
        if re.match("5\..*", name):
            benchmark[rank]["raytracer"     ] += value

    for rank in benchmark:
        benchmark[0]["partitioner"    ] = max(benchmark[0]["partitioner"    ], rank["partitioner"    ])
        benchmark[0]["dataset_loader" ] = max(benchmark[0]["dataset_loader" ], rank["dataset_loader" ])
        benchmark[0]["particle_tracer"] = max(benchmark[0]["particle_tracer"], rank["particle_tracer"])
        benchmark[0]["color_mapper"   ] = max(benchmark[0]["color_mapper"   ], rank["color_mapper"   ])
        benchmark[0]["raytracer"      ] = max(benchmark[0]["raytracer"      ], rank["raytracer"      ])

    return benchmark[0]

# Format: {"4": {"partitioner": 123.4, "dataset_loader": 567.8, "particle_tracer": 123.4, "color_mapper": 567.8, "raytracer": 123.4}, ...}
def parse_benchmark_scaling_multi(filepaths):
    benchmark = {}

    for filepath in filepaths:
        process = int(os.path.basename(filepath).split(".")[3])
        benchmark[process] = parse_benchmark_scaling(filepath)

    return benchmark