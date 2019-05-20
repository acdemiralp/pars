### Distributed local average workload balancing
Hybrid parallel particle advection approach which statically parallelizes over data and dynamically parallelizes over seeds. Each process loads their and their adjacents' blocks statically (7 blocks per process). The workload of adjacent processes are load balanced on each round, moving seeds from higher workload to lower workload processes. Since the destination process already preloaded the block the source process' seeds are defined in, no IO is performed at runtime. Since the local neighborhood workload computation is performed per process in a distributed manner and only involves basic arithmetic, it does not introduce significant overhead such as a KD-tree recomputation.

It does not guarantee assigning an equal amount of particles to the local neighborhood deterministically, due to the distributed nature of the load balancing step. Yet it guarantees to improve the load balancing in the local neighborhood.

This implementation utilizes MPI for distributed-memory and Intel TBB for shared-memory parallelism (hence also a hybrid in terms of parallelization strategy).

#### Stages
- Partitioner    : Divides the domain into blocks.
- Data loader    : Loads the blocks for this and adjacent processes.
- Seed generator : Generates seeds over the whole domain (with a stride).
- Particle tracer: Traces the particles.
- Color generator: Generates colors  from the vertices.
- Index generator: Generates indices from the vertices.

#### Notes
- Integral curves expect vertices/colors to not exceed `sizeof int32 / sizeof vector4`.