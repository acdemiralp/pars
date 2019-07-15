import csv
import os
import re

def parse_benchmark                  (filepath):
    benchmark = []
    with open(filepath, mode='r') as csv_file:
        csv_reader = csv.DictReader(csv_file)
        for row in csv_reader:
            benchmark.append(row)
    return benchmark

# Format: [[{"load_balancing": 456.7, "tracing": 123.4, "communication": 567.8}, ...]]
def parse_benchmark_gantt            (filepath):
    raw_benchmark = parse_benchmark(filepath)

    benchmark = []
    for row in raw_benchmark:
        rank  = int  (row["rank"])
        name  =       row["name"]
        value = float(row["iteration 0"])

        if rank >= len(benchmark):
            benchmark.extend([[]] * (rank + 1 - len(benchmark)))
        
        if (re.match("3\.1\..*\.0", name)):
            iteration = int(name.split(".")[2])
            if iteration >= len(benchmark[rank]):
                benchmark[rank].extend([{"load_balancing": 0.0, "tracing": 0.0, "communication": 0.0}] * (iteration + 1 - len(benchmark[rank])))
            benchmark[rank][iteration]["load_balancing"] += value
            
        if (re.match("3\.1\..*\.1", name) or
            re.match("3\.1\..*\.2", name) or
            re.match("3\.1\..*\.3", name) or
            re.match("3\.1\..*\.4", name)):
            iteration = int(name.split(".")[2])
            if iteration >= len(benchmark[rank]):
                benchmark[rank].extend([{"load_balancing": 0.0, "tracing": 0.0, "communication": 0.0}] * (iteration + 1 - len(benchmark[rank])))
            benchmark[rank][iteration]["tracing"] += value

        if (re.match("3\.1\..*\.5", name) or
            re.match("3\.1\..*\.6", name) or
            re.match("3\.1\..*\.7", name)):
            iteration = int(name.split(".")[2])
            if iteration >= len(benchmark[rank]):
                benchmark[rank].extend([{"load_balancing": 0.0, "tracing": 0.0, "communication": 0.0}] * (iteration + 1 - len(benchmark[rank])))
            benchmark[rank][iteration]["communication"] += value
            
    return benchmark

# Format:       {"data_loader": 567.8, "particle_tracer": 123.4, "color_mapper": 567.8, "ray_tracer": 123.4}
def parse_benchmark_scaling          (filepath):
    raw_benchmark = parse_benchmark(filepath)

    benchmark = []
    for row in raw_benchmark:
        rank  = int  (row["rank"])
        name  =       row["name"]
        value = float(row["iteration 0"])

        if rank >= len(benchmark):
            benchmark.extend([{
                "data_loader"     : 0.0, 
                "seed_generator"  : 0.0, 
                "particle_tracer" : 0.0, 
                "index_generator" : 0.0, 
                "color_generator" : 0.0, 
                "ray_tracer_setup": 0.0,
                "ray_tracer_trace": 0.0}] * (rank + 1 - len(benchmark)))
               
        if re.match("1\..*", name):
            benchmark[rank]["data_loader"     ] += value
        if re.match("2\..*", name):
            benchmark[rank]["seed_generator"  ] += value
        if re.match("3\..*", name):
            benchmark[rank]["particle_tracer" ] += value
        if re.match("4\..*", name):
            benchmark[rank]["index_generator" ] += value
        if re.match("5\..*", name):
            benchmark[rank]["color_generator" ] += value
        if re.match("6\..*1", name) or re.match("6\..*2", name):
            benchmark[rank]["ray_tracer_setup"] += value
        if re.match("6\..*3", name):
            benchmark[rank]["ray_tracer_trace"] += value

    for rank in benchmark:
        benchmark[0]["data_loader"     ] = max(benchmark[0]["data_loader"     ], rank["data_loader"     ])
        benchmark[0]["seed_generator"  ] = max(benchmark[0]["seed_generator"  ], rank["seed_generator"  ])
        benchmark[0]["particle_tracer" ] = max(benchmark[0]["particle_tracer" ], rank["particle_tracer" ])
        benchmark[0]["index_generator" ] = max(benchmark[0]["index_generator" ], rank["index_generator" ])
        benchmark[0]["color_generator" ] = max(benchmark[0]["color_generator" ], rank["color_generator" ])
        benchmark[0]["ray_tracer_setup"] = max(benchmark[0]["ray_tracer_setup"], rank["ray_tracer_setup"])
        benchmark[0]["ray_tracer_trace"] = max(benchmark[0]["ray_tracer_trace"], rank["ray_tracer_trace"])

    return benchmark[0]

# Format: {"4": {"data_loader": 567.8, "particle_tracer": 123.4, "color_mapper": 567.8, "ray_tracer": 123.4}, ...}
def parse_benchmark_node_scaling     (filepaths):
    benchmark = {}

    for filepath in filepaths:
        process = int(os.path.basename(filepath).split("_")[2][1:])
        benchmark[process] = parse_benchmark_scaling(filepath)

    return benchmark

# Format: {"4": {"data_loader": 567.8, "particle_tracer": 123.4, "color_mapper": 567.8, "ray_tracer": 123.4}, ...}
def parse_benchmark_processor_scaling(filepaths):
    benchmark = {}

    for filepath in filepaths:
        process = int(os.path.basename(filepath).split("_")[3][1:])
        benchmark[process] = parse_benchmark_scaling(filepath)

    return benchmark