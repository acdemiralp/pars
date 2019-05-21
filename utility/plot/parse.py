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
        
        if (re.match("3\.1\..*\.1", name) or
            re.match("3\.1\..*\.2", name) or
            re.match("3\.1\..*\.3", name) or
            re.match("3\.1\..*\.4", name)):
            iteration = int(name.split(".")[2])
            if iteration >= len(benchmark[rank]):
                benchmark[rank].extend([{"tracing": 0.0, "communication": 0.0}] * (iteration + 1 - len(benchmark[rank])))
            benchmark[rank][iteration]["tracing"]       += value

        if (re.match("3\.1\..*\.0", name) or
            re.match("3\.1\..*\.5", name) or
            re.match("3\.1\..*\.6", name) or
            re.match("3\.1\..*\.7", name)):
            iteration = int(name.split(".")[2])
            if iteration >= len(benchmark[rank]):
                benchmark[rank].extend([{"tracing": 0.0, "communication": 0.0}] * (iteration + 1 - len(benchmark[rank])))
            benchmark[rank][iteration]["communication"] += value
            
    return benchmark

# Format:       {"partitioner": 123.4, "data_loader": 567.8, "particle_tracer": 123.4, "color_mapper": 567.8, "ray_tracer": 123.4}
def parse_benchmark_scaling      (filepath):
    raw_benchmark = parse_benchmark(filepath)

    benchmark = []
    for row in raw_benchmark:
        rank  = int  (row["rank"])
        name  =       row["name"]
        value = float(row["iteration 0"])

        if rank >= len(benchmark):
            benchmark.extend([{
                "partitioner"    : 0.0, 
                "data_loader"    : 0.0, 
                "seed_generator" : 0.0, 
                "particle_tracer": 0.0, 
                "index_generator": 0.0, 
                "color_generator": 0.0, 
                "ray_tracer"     : 0.0}] * (rank + 1 - len(benchmark)))
        
        if re.match("0\..*", name):
            benchmark[rank]["partitioner"    ] += value          
        if re.match("1\..*", name):
            benchmark[rank]["data_loader"    ] += value
        if re.match("2\..*", name):
            benchmark[rank]["seed_generator" ] += value
        if re.match("3\..*", name):
            benchmark[rank]["particle_tracer"] += value
        if re.match("4\..*", name):
            benchmark[rank]["index_generator"] += value
        if re.match("5\..*", name):
            benchmark[rank]["color_generator"] += value
        if re.match("6\..*", name):
            benchmark[rank]["ray_tracer"     ] += value

    for rank in benchmark:
        benchmark[0]["partitioner"    ] = max(benchmark[0]["partitioner"    ], rank["partitioner"    ])
        benchmark[0]["data_loader"    ] = max(benchmark[0]["data_loader"    ], rank["data_loader"    ])
        benchmark[0]["seed_generator" ] = max(benchmark[0]["seed_generator" ], rank["seed_generator" ])
        benchmark[0]["particle_tracer"] = max(benchmark[0]["particle_tracer"], rank["particle_tracer"])
        benchmark[0]["index_generator"] = max(benchmark[0]["index_generator"], rank["index_generator"])
        benchmark[0]["color_generator"] = max(benchmark[0]["color_generator"], rank["color_generator"])
        benchmark[0]["ray_tracer"     ] = max(benchmark[0]["ray_tracer"     ], rank["ray_tracer"     ])

    return benchmark[0]

# Format: {"4": {"partitioner": 123.4, "data_loader": 567.8, "particle_tracer": 123.4, "color_mapper": 567.8, "ray_tracer": 123.4}, ...}
def parse_benchmark_scaling_multi(filepaths):
    benchmark = {}

    for filepath in filepaths:
        process = int(os.path.basename(filepath).split(".")[3])
        benchmark[process] = parse_benchmark_scaling(filepath)

    return benchmark