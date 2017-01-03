#python scirpts 
##Utilities
- [util_helper.py](main_files/util_helper.py), util codes is put in this file
- [yche_exp.py](demo_files/yche_exp.py), some experimental codes is put in this file

##Demo Codes: 

content | detail
--- | ---
[gexpmq.py](demo_files/gexpmq.py) | A demonstration of the Gauss-Seidel coordinate relaxation scheme
[hr_relax](demo_files/hr_relax.py) | A demonstration of a relaxation method for computing a heat-kernel based community


##Heat Kernel Growth(With Relax)
- Code: [hr_grow.py](main_files/hr_grow.py), code is refactored by Yulin CHE to make it more easier to understand

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

##Personalized Page  Rank
- Code: [ppr.py](main_files/ppr.py), code is refactored by Yulin CHE to make it more easier to understand