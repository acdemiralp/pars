#include <array>
#include <cstddef>
#include <fstream>
#include <string>
#include <vector>

std::string settings_template = R"({
  "dataset_filepath"               : "$1",

  "seed_generation_stride"         : [ $2, $2, $2 ],
  "seed_generation_iterations"     : $3,
  
  "particle_tracing_integrator"    : "runge_kutta_4",
  "particle_tracing_step_size"     : 0.5,
  "particle_tracing_load_balance"  : $4,
  
  "color_generation_mode"          : "hsv_constant_s",
  "color_generation_free_parameter": 0.75,
  
  "raytracing_camera_position"     : [ $5, $6, $7 ],
  "raytracing_camera_forward"      : [ 0.0, 0.0, 1.0 ],
  "raytracing_camera_up"           : [ 0.0, 1.0, 0.0 ],
  "raytracing_image_size"          : [ 1080, 1920 ],
  "raytracing_streamline_radius"   : 0.1,
  "raytracing_iterations"          : 1
})"; // $1 dataset filepath, $2 seed stride x/y/z, $3 seed iterations, $4 load balancing, $5/$6/$7 camera x/y/z.

std::string slurm_script_template = R"(#!/bin/bash
#SBATCH --job-name=$1
#SBATCH --output=$1.log
#SBATCH --time=00:15:00
#SBATCH --mem=128000M
#SBATCH --nodes=$2
#SBATCH --cpus-per-task=$3
#SBATCH --ntasks-per-core=1
#SBATCH --ntasks-per-node=1
#SBATCH --account=jinm11

module load CMake/3.14.0
module load Intel/2019.3.199-GCC-8.3.0
module load IntelMPI/2019.3.199
module load Boost/1.69.0-Python-2.7.16
module load HDF5/1.10.5
module load imkl/2019.3.199
module load tbb/2019.4.199

srun --mpi=pmi2 /p/home/jusers/demiralp1/jureca/source/pars/build/pars_benchmark/pars_benchmark $3 ./$1.json
)"; // $1 name, $2 nodes, $3 processors.

struct configuration
{
  std::string              dataset_filepath       ;
  std::size_t              dataset_scale          ; // Specifies how up-scaled the data with the given filepath is.
  std::vector<std::size_t> nodes                  ; // Combinatorial.
  std::vector<std::size_t> processors             ; // Combinatorial.
  std::vector<std::size_t> seed_generation_strides; // Combinatorial.
  std::vector<std::size_t> seed_iterations        ; // Combinatorial.
  std::vector<bool>        load_balancing         ; // Combinatorial.
  std::array<float, 3>     camera_position        ;
};

int main(int argc, char** argv)
{
  std::vector<configuration> configurations
  {
    configuration
    {
      "/p/project/cjinm11/Private/demiralp1/data/pli/msa/MSA0309_s0536-0695_c.h5"     , // ~4 GB
      1,
      {4, 8, 16, 24, 32, 64, 128, 256}, // Even a single machine can handle the non-scaled dataset.
      {4, 8, 16, 24},
      {4, 8, 16, 32, 64},
      {128, 1024},
      {true, false},
      {500.0, 750.0, -1250.0}
    },
    configuration
    {
      "/p/project/cjinm11/Private/demiralp1/data/pli/msa/MSA0309_s0536-0695_c_s2.h5"  , // ~27 GB
      2,
      {4, 8, 16, 24, 32, 64, 128, 256},
      {4, 8, 16, 24},
      {4, 8, 16, 32, 64},
      {128, 1024},
      {true, false},
      {1000.0, 1500.0, -2500.0}
    },
    configuration
    {
      "/p/project/cjinm11/Private/demiralp1/data/pli/msa/MSA0309_s0536-0695_c_s4.h5"  , // ~209 GB
      4,
      {4, 8, 16, 24, 32, 64, 128, 256},
      {4, 8, 16, 24},
      {4, 8, 16, 32, 64},
      {128, 1024},
      {true, false},
      {2000.0, 3000.0, -5000.0}
    },
    configuration
    {
      "/p/project/cjinm11/Private/demiralp1/data/pli/msa/MSA0309_s0536-0695_c_s8.h5"  , // ~1.7 TB
      8,
      {      16, 24, 32, 64, 128, 256},
      {4, 8, 16, 24},
      {4, 8, 16, 32, 64},
      {128, 1024},
      {true, false},
      {4000.0, 6000.0, -10000.0}
    },
    configuration
    {
      "/p/scratch/cjinm11/demiralp1/data/pli/msa/MSA0309_s0536-0695_c_s10.h5"         , // ~3.3 TB
      10,
      {              32, 64, 128, 256},
      {4, 8, 16, 24},
      {4, 8, 16, 32, 64},
      {128, 1024},
      {true, false},
      {5000.0, 7500.0, -12500.0}
    },
    configuration
    {
      "/p/scratch/cjinm11/demiralp1/data/pli/msa/MSA0309_s0536-0695_c_s16.h5"         , // ~14 TB
      16,
      {                      128, 256},
      {4, 8, 16, 24},
      {4, 8, 16, 32, 64},
      {128, 1024},
      {true, false},
      {8000.0, 12000.0, -20000.0}
    }
  };

  std::vector<std::string> scripts;

  // Create setting - script pairs from the configurations.
  {
    for (auto& configuration          : configurations                       ) {
    for (auto& node                   : configuration.nodes                  ) {
    for (auto& processor              : configuration.processors             ) {
    for (auto& seed_generation_stride : configuration.seed_generation_strides) {
    for (auto& seed_iteration         : configuration.seed_iterations        ) {
    for (auto& load_balance           : configuration.load_balancing         ) {
      auto name = std::string("benchmark") +
        "_sc" + std::to_string(configuration.dataset_scale) +
        "_n"  + std::to_string(node) +
        "_p"  + std::to_string(processor) +
        "_st" + std::to_string(seed_generation_stride) +
        "_i"  + std::to_string(seed_iteration) +
        "_lb" + (load_balance ? "1" : "0");

      // Create the settings.
      auto settings = settings_template;
      while (settings.find("$1") != std::string::npos) settings.replace(settings.find("$1"), 2, configuration.dataset_filepath);
      while (settings.find("$2") != std::string::npos) settings.replace(settings.find("$2"), 2, std::to_string(seed_generation_stride));
      while (settings.find("$3") != std::string::npos) settings.replace(settings.find("$3"), 2, std::to_string(seed_iteration));
      while (settings.find("$4") != std::string::npos) settings.replace(settings.find("$4"), 2, load_balance ? "true" : "false");
      while (settings.find("$5") != std::string::npos) settings.replace(settings.find("$5"), 2, std::to_string(configuration.camera_position[0]));
      while (settings.find("$6") != std::string::npos) settings.replace(settings.find("$6"), 2, std::to_string(configuration.camera_position[1]));
      while (settings.find("$7") != std::string::npos) settings.replace(settings.find("$7"), 2, std::to_string(configuration.camera_position[2]));

      std::ofstream settings_stream(name + ".json");
      settings_stream << settings;
      settings_stream.close();

      // Create the script.
      auto script = slurm_script_template;
      while (script.find("$1") != std::string::npos) script.replace(script.find("$1"), 2, name);
      while (script.find("$2") != std::string::npos) script.replace(script.find("$2"), 2, std::to_string(node));
      while (script.find("$3") != std::string::npos) script.replace(script.find("$3"), 2, std::to_string(processor));

      std::ofstream script_stream(name + ".sh");
      script_stream << script;
      script_stream.close();

      scripts.push_back(name + ".sh");
    }}}}}}
  }

  // Create master script, batching all scripts.
  {
    std::string master_script = "#!/bin/bash\n";
    for (auto& script : scripts)
      master_script += "sbatch ./" + script + "\n";

    std::ofstream stream("benchmark_master.sh");
    stream << master_script;
    stream.close();
  }

  return 0;
}