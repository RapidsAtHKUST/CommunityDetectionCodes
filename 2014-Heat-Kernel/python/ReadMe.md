#python scirpts 
##Heat Kernel Growth
- Code: [hr_grow.py](hr_grow.py), code is refactored by Yulin CHE to make it more easier to understand

- Authors:

> by Kyle Kloster and David F. Gleich
> supported by NSF award CCF-1149756.

- Description:

> This demo shows our algorithm running on the Twitter graph.
> Our other codes are available from David's website:
>
>   https://www.cs.purdue.edu/homes/dgleich/codes/hkgrow/
>
> To use this demo, you need pylibbvg graph by David Gleich, Wei-Yen Day, and
> Yongyang Yu (heavily based on webgraph by Sebastiano Vigna -- he did all the
> fundamental work!) as well as the symmetrized version of the twitter graph.
>
> If you are on a mac or linux, run
>
>   pip install pylibbvg
>   wget https://www.cs.purdue.edu/homes/dgleich/data/twitter-2010-symm.graph
>   wget https://www.cs.purdue.edu/homes/dgleich/data/twitter-2010-symm.offsets
>   wget https://www.cs.purdue.edu/homes/dgleich/data/twitter-2010-symm.properties
>
> Then, as long as these files are in your directory, our code will run!
>
> You need about 4-5GB of space and memory to run this demo.