# New Findings About Social Network Analysis (From Statistical Perspective)

1. Static Network Random Generation Mechanism
  - Community Detection  
    Hot topic : stochastic block model,  
    Recommended Paper :  
      - **Spectral clustering**
        - Rohe K, Chatterjee S, Yu B. [Spectral clustering and the high-dimensional stochastic blockmodel[J]](http://www.jstor.org/stable/23033587?seq=1#page_scan_tab_contents). The Annals of Statistics, 2011: 1878-1915. (The most Influential spectral clustering paper)  
        - Lei J, Rinaldo A. [Consistency of spectral clustering in stochastic block models[J]. The Annals of Statistics](http://projecteuclid.org/euclid.aos/1418135620), 2014, 43(1): 215-237.  
        - Qin T, Rohe K. [Regularized spectral clustering under the degree-corrected stochastic blockmodel](http://papers.nips.cc/paper/5099-regularized-spectral-clustering-under-the-degree-corrected-stochastic-blockmodel)[C]//Advances in Neural Information Processing Systems. 2013: 3120-3128.  
      - **SDP**  
        - Amini A A, [Levina E. On semidefinite relaxations for the block model](http://arxiv.org/abs/1406.5647)[J]. arXiv preprint arXiv:1406.5647, 2014. (Influential One)    
        - Cai T T, Li X. [Robust and computationally feasible community detection in the presence of arbitrary outlier nodes](http://projecteuclid.org/euclid.aos/1431695637)[J]. The Annals of Statistics, 2015, 43(3): 1027-1059.  
      - **Others**
        - Jin J. [Fast community detection by SCORE[J]](http://projecteuclid.org/euclid.aos/1416322036). The Annals of Statistics, 2015, 43(1): 57-89.  
        - Amini A A, Chen A, Bickel P J, et al. [Pseudo-likelihood methods for community detection in large sparse networks[J]](http://projecteuclid.org/euclid.aos/1382547514). The Annals of Statistics, 2013, 41(4): 2097-2122. (Good Paper)    
        - Gao C, Ma Z, Zhang A Y, et al. [Achieving Optimal Misclassification Proportion in Stochastic Block Model[J]](http://arxiv.org/abs/1505.03772). arXiv preprint arXiv:1505.03772, 2015. (spectral initialization, and then majority vote)
  - Link Prediction  
  Hot topic : edge probability matrix,  
  Application : drug traffic analysis,

2. Relationship Between Static Network and Node Covariates  
  - Example: follow relationship along with user profiles such as gender, hobby, etc(Covariates).
  - Topic1 : use network to analyze node covariates  
  - Topic2 : combine node covariates clustering structure with network clustering structure, enhance accuracy in finite sample  
  (Since in reality, network is small, which could not guarantee enough nodes for 100% accuracy.)  
  - Topic3 : explorative tools for analysis

3. Dynamic Network Analysis
  - Example : tweet, weibo, left-message, share and response in webchat, edges have timestamps  
  - Topic : assume stability(problem with event-based property)  

- References
  - Jack Diamond, Overview About Social Network Analysis, https://www.zhihu.com/question/28939731  
  - Jack Diamond, Classical Methods for Community Detection, https://www.zhihu.com/question/29042018  
