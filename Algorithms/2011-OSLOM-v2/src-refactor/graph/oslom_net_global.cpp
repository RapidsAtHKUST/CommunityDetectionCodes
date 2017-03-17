//
// Created by cheyulin on 3/17/17.
//
#include "oslom_net_global.h"

oslom_net_global::oslom_net_global(map<int, map<int, pair<int, double> > > &A) : oslomnet_evaluate(A) {
}

oslom_net_global::oslom_net_global(int_matrix &b, deque<deque<pair<int, double> >> &c, deque<int> &d)
        : oslomnet_evaluate(b, c, d) {
}

oslom_net_global::oslom_net_global(string a) : oslomnet_evaluate(a) {
}

int oslom_net_global::try_to_merge_discarded(int_matrix &discarded, int_matrix &good_modules_to_prune,
                                             int_matrix &new_discarded, deque<double> &bscores_good) {
    new_discarded.clear();
    /* this function is implemented to check if merging discarded modules we can get significant clusters */
    /* discarded is the input, the results are appended, except new_discarded which is cleared */
    if (discarded.size() == 0)
        return -1;
    if (paras.print_flag_subgraph)
        cout << "checking unions of not significant modules, modules to check: " << discarded.size() << endl;
    module_collection discarded_modules(dim);
    for (int i = 0; i < int(discarded.size()); i++)
        discarded_modules.insert(discarded[i], 1);
    map<int, map<int, pair<int, double> > > neigh_weight_s;        // this maps the module id into the neighbor module ids and weights
    set_upper_network(neigh_weight_s, discarded_modules);
    if (neigh_weight_s.size() == 0)
        return -1;
    oslom_net_global community_net(neigh_weight_s);
    //community_net.draw("community_net");
    int_matrix M_raw;        /* M_raw contains the module_id of discarded_modules */
    community_net.collect_raw_groups_once(M_raw);
    /*cout<<"community_net partition: "<<endl;
    printm(M_raw);
    cout<<"trying unions of discarded modules... "<<endl;*/
    for (int i = 0; i < int(M_raw.size()); i++)
        if (M_raw[i].size() > 1) {
            set<int> l1;
            for (int j = 0; j < int(M_raw[i].size()); j++)
                deque_to_set_app(discarded_modules.modules[M_raw[i][j]], l1);
            deque<int> _M_i_;
            set_to_deque(l1, _M_i_);
            //cout<<"merged discarded: "<<M_raw[i].size()<<endl;
            //print_id(_M_i_, cout);
            deque<int> l;
            double bscore = CUP_check(_M_i_, l);
            if (l.size() > 0) {
                good_modules_to_prune.push_back(l);
                bscores_good.push_back(bscore);
            } else
                new_discarded.push_back(_M_i_);
        }
    return 0;
}

void oslom_net_global::get_single_trial_partition(int_matrix &good_modules_to_prune, deque<double> &bscores_good) {
    /* this function collects significant modules in two steps:
       1. using collect_raw_groups_once and cleaning
       2. putting together discarded modules
       the results are appended in the input data	*/
    int_matrix discarded;
    int_matrix M;
    //*****************************************************************************
    collect_raw_groups_once(M);
    //cout<<"single_gather"<<endl;
    //print_id(M, cout);
    //*****************************************************************************
    UI total_nodes_tested = 0;
    for (UI i = 0; i < M.size(); i++) {
        if (paras.print_flag_subgraph && i % 100 == 0) {
            cout << "checked " << i << " modules " << good_modules_to_prune.size()
                 << " were found significant.  Modules to check: " << M.size() - i << ". Percentage nodes done: "
                 << double(total_nodes_tested) / dim << endl;
        }
        /*if(paras.print_flag_subgraph && M[i].size()>1000)
            cout<<"M[i].size(): "<<M[i].size()<<endl;*/
        total_nodes_tested += M[i].size();
        deque<int> l;
        double bscore;
        if (M[i].size() < 1000)
            bscore = group_inflation(M[i], l);
        else        /* if M[i] is big enough the check is faster to save time */
            bscore = CUP_both(M[i], l);
        if (l.size() > 0) {
            good_modules_to_prune.push_back(l);
            bscores_good.push_back(bscore);
        } else
            discarded.push_back(M[i]);
    }
    if (paras.print_flag_subgraph)
        cout << "significance check done " << endl << endl << endl;
    //*****************************************************************************
    int_matrix new_discarded;
    try_to_merge_discarded(discarded, good_modules_to_prune, new_discarded, bscores_good);
    discarded = new_discarded;
    try_to_merge_discarded(discarded, good_modules_to_prune, new_discarded, bscores_good);
    if (paras.print_flag_subgraph)
        cout << "checking unions of not significant modules done " << endl << endl << endl;
    /* actually here it is also possible to check the new_discarded modules more than twice but I believe this should be enough */
    /* in principle one could do while(try_to_merge_discarded(...)!=-1) */
}

void oslom_net_global::single_gather(int_matrix &good_modules_to_prune, deque<double> &bscores_good, int runs = 1) {
    good_modules_to_prune.clear();
    bscores_good.clear();
    for (int i = 0; i < runs; i++)
        get_single_trial_partition(good_modules_to_prune, bscores_good);
}

void oslom_net_global::from_matrix_to_module_collection(int_matrix &good_modules_to_prune, DD &bscores_good,
                                                        module_collection &minimal_modules) {
    check_minimality_all(good_modules_to_prune, bscores_good, minimal_modules);
    cout << "***************************************************************************" << endl;
    cout << "MINIMALITY CHECK DONE" << endl;
    check_unions_and_overlap(minimal_modules);
    cout << "***************************************************************************" << endl;
    cout << "CHECK UNIONS AND SIMILAR MODULES DONE" << endl;
}

void oslom_net_global::get_cover(module_collection &minimal_modules) {
    /* this function collects the modules using single_gather */
    /* then the good modules are inserted in minimal_modules afetr the check minimality */
    paras.print_flag_subgraph = true;
    int_matrix good_modules_to_prune;
    deque<double> bscores_good;
    single_gather(good_modules_to_prune, bscores_good);
    cout << "***************************************************************************" << endl;
    cout << "COLLECTING SIGNIFICANT MODULES DONE" << endl << endl;
    from_matrix_to_module_collection(good_modules_to_prune, bscores_good, minimal_modules);
}

void oslom_net_global::check_minimality_all(int_matrix &A, DD &bss, module_collection &minimal_modules) {
    paras.print_flag_subgraph = false;
    {
        /* simplifying A*/
        module_collection suggestion_mall(dim);
        for (UI i = 0; i < A.size(); i++)
            suggestion_mall.insert(A[i], 1);
        suggestion_mall.erase_included();
        suggestion_mall.set_partition(A);
    }
    int counter = 0;
    while (A.size() > 0) {
        int_matrix suggestion_matrix;
        deque<double> suggestion_bs;
        check_minimality_matrix(A, bss, minimal_modules, suggestion_matrix, suggestion_bs, counter);
        module_collection suggestion_mall(dim);
        for (UI i = 0; i < suggestion_matrix.size(); i++)
            suggestion_mall.insert(suggestion_matrix[i], 1);
        suggestion_mall.erase_included();
        suggestion_mall.set_partition(A);
        bss = suggestion_bs;
        ++counter;
    }
}

void oslom_net_global::check_minimality_matrix(int_matrix &A, DD &bss, module_collection &minimal_modules,
                                               int_matrix &suggestion_matrix, deque<double> &suggestion_bs,
                                               int counter) {
    if (A.size() > 4)
        cout << "minimality check: " << A.size() << " modules to check, run: " << counter << endl;
    if (counter < paras.minimality_stopper) {
        for (UI i = 0; i < A.size(); i++) {
            /*if(i%100==0)
                cout<<"checked: "<<i<<" modules.  Modules to check... "<<A.size() - i<<endl;*/
            check_minimality(A[i], bss[i], minimal_modules, suggestion_matrix, suggestion_bs);
        }
    } else {
        for (UI i = 0; i < A.size(); i++)
            minimal_modules.insert(A[i], bss[i]);
    }
}

bool oslom_net_global::check_minimality(deque<int> &group, double &bs_group, module_collection &minimal_modules,
                                        int_matrix &suggestion_matrix, deque<double> &suggestion_bs) {
    /*	this function checks the minimality of group
        minimality means that group doesn't have internal structures up to a factor coverage_percentage_fusion_left
        returns true is group is inserted in minimal_modules	*/
    int_matrix subM;
    deque<double> bss;
    {    //******************  module_subgraph stuff   ******************
        deque<deque<int>> link_per_node;
        deque<deque<pair<int, double> >> weights_per_node;
        set_subgraph(group, link_per_node, weights_per_node);
        oslom_net_global module_subgraph(link_per_node, weights_per_node, group);
        DD bscores_good_temp;
        module_subgraph.single_gather(subM, bscores_good_temp);
        for (int i = 0; i < int(subM.size()); i++) {
            module_subgraph.deque_id(subM[i]);
            deque<int> grbe;
            bss.push_back(CUP_check(subM[i], grbe));
            subM[i] = grbe;
        }            /* so now you know these modules are cleaned (but you are not sure they are minimal) */
    }   //******************  module_subgraph stuff   ******************
    for (UI i = 0; i < subM.size(); i++)
        if (subM[i].size() == group.size()) {
            minimal_modules.insert(group, bs_group);
            return true;
        }
    set<int> a;
    for (UI i = 0; i < subM.size(); i++)
        for (int j = 0; j < int(subM[i].size()); j++)
            a.insert(subM[i][j]);
    if (a.size() > paras.coverage_percentage_fusion_left * group.size()) {
        /* this means the group cannot be accepted */
        for (UI i = 0; i < subM.size(); i++)
            if (subM[i].size() > 0) {
                suggestion_matrix.push_back(subM[i]);
                suggestion_bs.push_back(bss[i]);
            }
        return false;
    } else {
        minimal_modules.insert(group, bs_group);
        return true;
    }
}

void oslom_net_global::print_modules(bool not_homeless, string tp, module_collection &Mcoll) {
    char b[1000];
    cast_string_to_char(tp, b);
    ofstream out1(b);
    print_modules(not_homeless, out1, Mcoll);
}

void oslom_net_global::print_modules(bool not_homeless, ostream &out1, module_collection &Mcoll) {
    int nmod = 0;
    for (map<int, double>::iterator itm = Mcoll.module_bs.begin(); itm != Mcoll.module_bs.end(); itm++)
        if (Mcoll.modules[itm->first].size() > 1)
            nmod++;
    cout << "******** module_collection ******** " << nmod << " modules. writing... " << endl;
    deque<int> netlabs;
    for (int i = 0; i < dim; i++)
        netlabs.push_back(id_of(i));
    Mcoll.print(out1, netlabs, not_homeless);
    cout << "DONE   ****************************" << endl;
}

void oslom_net_global::load(string filename, module_collection &Mall) {
    // this function is to read a file in the tp-format
    cout << "getting partition from tp-file: " << filename << endl;
    deque<double> bss;
    deque<deque<int>> A;
    get_partition_from_file_tp_format(filename, A, bss);
    translate(A);
    cout << A.size() << " groups found" << endl;
    cout << bss.size() << " bss found" << endl;
    for (UI ii = 0; ii < A.size(); ii++) {
        //cout<<"inserting group number "<<ii<<" size: "<<A[ii].size()<<endl;
        Mall.insert(A[ii], bss[ii]);
    }
}

void oslom_net_global::get_covers(string cover_file, int &soft_partitions_written, int gruns) {
    /* this function is to collect different covers of the network
       they are written in file cover_file (appended)
       their number is added to soft_partitions_written */
    char b[1000];
    cast_string_to_char(cover_file, b);
    ofstream out1(b, ios::app);
    for (int i = 0; i < gruns; i++) {
        cout << "***************************************************************** RUN: #" << i + 1 << endl;
        module_collection Mcoll(dim);
        get_cover(Mcoll);
        if (Mcoll.size() > 0) {
            print_modules(true, out1, Mcoll);                // not homeless nodes
            soft_partitions_written++;
        }
    }
    if (paras.value) {
        module_collection Mcoll(dim);
        hint(Mcoll, paras.file2);
        if (Mcoll.size() > 0) {
            print_modules(true, out1, Mcoll);                // not homeless nodes
            soft_partitions_written++;
        }
    }
    if (paras.value_load) {
        module_collection Mcoll(dim);
        load(paras.file_load, Mcoll);
        if (Mcoll.size() > 0) {
            print_modules(true, out1, Mcoll);                // not homeless nodes
            soft_partitions_written++;
        }
    }
}

void oslom_net_global::ultimate_cover(string cover_file, int soft_partitions_written, string final_cover_file) {
    cout << "pruning all the modules collected. Partitions found: " << soft_partitions_written << endl;
    module_collection Mall(dim);
    load(cover_file, Mall);
    if (soft_partitions_written > 1)
        check_unions_and_overlap(Mall, true);
    cout << "checking homeless nodes" << endl;
    if (paras.homeless_anyway == false) {
        try_to_assign_homeless(Mall, false);
    } else {
        deque<int> homel;
        Mall.homeless(homel);
        UI before_procedure = homel.size();
        while (homel.size() > 0) {
            cout << "assigning homeless nodes. Homeless at this point: " << before_procedure << endl;
            try_to_assign_homeless(Mall, true);
            Mall.homeless(homel);
            if (homel.size() >= before_procedure)
                break;
            before_procedure = homel.size();
        }
    }
    Mall.fill_gaps();
    cout << "writing final solution in file " << final_cover_file << endl;
    print_modules(false, final_cover_file, Mall);                // homeless nodes printed
}

void oslom_net_global::hint(module_collection &minimal_modules, string filename) {
    int_matrix good_modules_to_prune;
    deque<double> bscores_good;
    cout << "getting partition from file: " << filename << endl;
    int_matrix A;
    get_partition_from_file(filename, A);
    translate(A);
    cout << A.size() << " groups found" << endl;
    for (int ii = 0; ii < int(A.size()); ii++) {
        deque<int> group;
        cout << "processing group number " << ii << " size: " << A[ii].size() << endl;
        double bcu = CUP_both(A[ii], group);
        if (group.size() > 0) {
            good_modules_to_prune.push_back(group);
            bscores_good.push_back(bcu);
        } else
            cout << "bad group" << endl;
    }
    cout << "***************************************************************************" << endl;
    from_matrix_to_module_collection(good_modules_to_prune, bscores_good, minimal_modules);
}

void oslom_net_global::print_statistics(ostream &outt, module_collection &Mcoll) {
    int nmod = 0;
    UI cov = 0;
    for (map<int, double>::iterator itm = Mcoll.module_bs.begin(); itm != Mcoll.module_bs.end(); itm++)
        if (Mcoll.modules[itm->first].size() > 1) {
            nmod++;
            cov += Mcoll.modules[itm->first].size();
        }
    deque<int> homel;
    Mcoll.homeless(homel);
    outt << "number of modules: " << nmod << endl;
    outt << "number of covered nodes: " << dim - homel.size() << " fraction of homeless nodes: "
         << double(homel.size()) / dim << endl;
    outt << "average number of memberships of covered nodes: " << double(cov) / (dim - homel.size()) << endl;
    outt << "average community size: " << double(cov) / nmod << endl;
    print_degree_of_homeless(homel, outt);
}

int oslom_net_global::check_intersection(module_collection &Mcoll) {
    paras.print_flag_subgraph = false;
    deque<int> to_check;
    for (map<int, double>::iterator itM = Mcoll.module_bs.begin(); itM != Mcoll.module_bs.end(); itM++)
        to_check.push_back(itM->first);
    return check_intersection(to_check, Mcoll);
}

int oslom_net_global::check_intersection(deque<int> &to_check, module_collection &Mcoll) {
    set<pair<int, int> > pairs_to_check;
    for (deque<int>::iterator itM = to_check.begin(); itM != to_check.end(); itM++)
        if (Mcoll.module_bs.find(*itM) != Mcoll.module_bs.end()) {
            deque<int> &c = Mcoll.modules[*itM];
            map<int, int> com_ol;                        // it maps the index of the modules into the overlap (overlap=number of overlapping nodes)
            for (UI i = 0; i < c.size(); i++)
                for (set<int>::iterator itj = Mcoll.memberships[c[i]].begin();
                     itj != Mcoll.memberships[c[i]].end(); itj++)
                    int_histogram(*itj, com_ol);
            for (map<int, int>::iterator cit = com_ol.begin(); cit != com_ol.end(); cit++)
                if (cit->first != *itM) {
                    if (double(cit->second) / min(Mcoll.modules[cit->first].size(), c.size()) >
                        paras.check_inter_p) {        // they have a few nodes in common
                        pairs_to_check.insert(make_pair(min(*itM, cit->first), max(*itM, cit->first)));
                    }
                }
        }
    return fusion_intersection(pairs_to_check, Mcoll);
}

int oslom_net_global::fusion_intersection(set<pair<int, int> > &pairs_to_check, module_collection &Mcoll) {
    cout << "pairs to check: " << pairs_to_check.size() << endl;
    deque<int> new_insertions;
    for (set<pair<int, int> >::iterator ith = pairs_to_check.begin(); ith != pairs_to_check.end(); ith++)
        if (ith->first < ith->second)
            if (Mcoll.module_bs.find(ith->first) != Mcoll.module_bs.end())
                if (Mcoll.module_bs.find(ith->second) != Mcoll.module_bs.end()) {
                    //		first, you need to check if both the modules in the pair are still in mcoll
                    deque<int> &a1 = Mcoll.modules[ith->first];
                    deque<int> &a2 = Mcoll.modules[ith->second];
                    int min_s = min(a1.size(), a2.size());
                    deque<int> group_intsec;
                    set_intersection(a1.begin(), a1.end(), a2.begin(), a2.end(), back_inserter(group_intsec));
                    //		if they are, you need to check if they are not almost equal.
                    if (double(group_intsec.size()) / min_s >= paras.coverage_inclusion_module_collection) {
                        int em = ith->first;
                        if (a1.size() < a2.size())
                            em = ith->second;
                        else if (a1.size() == a2.size() && Mcoll.module_bs[ith->first] > Mcoll.module_bs[ith->second])
                            em = ith->second;
                        Mcoll.erase(em);
                    } else
                        decision_fusion_intersection(ith->first, ith->second, new_insertions, Mcoll,
                                                     double(group_intsec.size()) / min_s);
                }
    if (new_insertions.size() > 0)
        return check_intersection(new_insertions, Mcoll);
    return 0;
}

bool
oslom_net_global::decision_fusion_intersection(int ai1, int ai2, deque<int> &new_insertions, module_collection &Mcoll,
                                               double prev_over_percentage) {
    deque<int> &a1 = Mcoll.modules[ai1];
    deque<int> &a2 = Mcoll.modules[ai2];
    deque<int> group;
    set_union(a1.begin(), a1.end(), a2.begin(), a2.end(), back_inserter(group));
    if (int(group.size()) != dim) {            //******************  sub_graph_module stuff   ******************
        deque<deque<int> > link_per_node;
        deque<deque<pair<int, double> > > weights_per_node;
        set_subgraph(group, link_per_node, weights_per_node);
        oslom_net_global sub_graph_module(link_per_node, weights_per_node, group);
        deque<deque<int> > A;
        A.push_back(a1);
        A.push_back(a2);
        sub_graph_module.translate(A);
        deque<int> grc1;
        double bs = sub_graph_module.CUP_check(A[0], grc1);
        deque<int> grc2;
        bs = sub_graph_module.CUP_check(A[1], grc2);
        deque<int> unions_grcs;
        set_union(grc1.begin(), grc1.end(), grc2.begin(), grc2.end(), back_inserter(unions_grcs));
        if (unions_grcs.size() <= paras.coverage_percentage_fusion_or_submodules *
                                  group.size()) {        // in such a case you can take the fusion (if it's good)
            /* actually the right check should be  unions_grcs.size() > paras.coverage_percentage_fusion_or_submodules*group_2.size()
               but this would require more time - it should not make a big difference anyway */
            deque<int> group_2;
            bs = CUP_check(group, group_2);
            if (group_2.size() > paras.coverage_percentage_fusion_left * group.size()) {
                Mcoll.erase(ai1);
                Mcoll.erase(ai2);
                int_matrix _A_;
                DD _bss_;
                _A_.push_back(group_2);
                _bss_.push_back(bs);
                check_minimality_all(_A_, _bss_, Mcoll);
                return true;
            } else
                return false;
        }
        sub_graph_module.deque_id(grc1);
        sub_graph_module.deque_id(grc2);
        deque<int> cg1;
        double bs__1 = CUP_check(grc1, cg1);
        deque<int> cg2;
        double bs__2 = CUP_check(grc2, cg2);
        deque<int> inters;
        set_intersection(cg1.begin(), cg1.end(), cg2.begin(), cg2.end(), back_inserter(inters));
        deque<int> unions;
        set_union(cg1.begin(), cg1.end(), cg2.begin(), cg2.end(), back_inserter(unions));
        if (double(inters.size()) / min(cg1.size(), cg2.size()) < prev_over_percentage - 1e-4) {
            if (cg1.size() > 0 && cg2.size() > 0 &&
                (unions.size() > paras.coverage_percentage_fusion_left * group.size())) {
                Mcoll.erase(ai1);
                Mcoll.erase(ai2);
                int newi;
                Mcoll.insert(cg1, bs__1, newi);
                new_insertions.push_back(newi);
                Mcoll.insert(cg2, bs__2, newi);
                new_insertions.push_back(newi);
                //cout<<"pruned module"<<endl;
                return true;
            }
        }
    }
    return false;
}

void from_int_matrix_and_deque_to_deque(int_matrix &its_submodules, const DI &A, DI &group) {
    // it merges A and its_submodules in group
    set<int> all_the_groups;
    for (UI i = 0; i < its_submodules.size(); i++) {
        for (UI j = 0; j < its_submodules[i].size(); j++)
            all_the_groups.insert(its_submodules[i][j]);
    }
    for (UI i = 0; i < A.size(); i++)
        all_the_groups.insert(A[i]);
    set_to_deque(all_the_groups, group);
}

bool oslom_net_global::fusion_module_its_subs(const deque<int> &A, deque<deque<int>> &its_submodules) {
    // A is supposed to be a good cluster
    // return true if A won against its submodules
    // ******************************************
    if (its_submodules.size() < 2)
        return true;
    DI group;
    from_int_matrix_and_deque_to_deque(its_submodules, A, group);
    {    //******************  sub_graph_module stuff   ******************
        deque<deque<int>> link_per_node;
        deque<deque<pair<int, double> > > weights_per_node;
        set_subgraph(group, link_per_node, weights_per_node);
        oslom_net_global sub_graph_module(link_per_node, weights_per_node, group);
        sub_graph_module.translate(its_submodules);
        //------------------------------------ cleaning up submodules --------------------------
        module_collection sub_mall(sub_graph_module.dim);
        for (UI i = 0; i < its_submodules.size(); i++)
            sub_mall.insert(its_submodules[i], 1e-3);
        sub_mall.set_partition(its_submodules);
        /*
        cout<<"group*************************************************"<<endl;
        print_id(group, cout);
        cout<<"A"<<endl;
        print_id(A, cout);
        cout<<"fusion_module_its_subs"<<endl;
        print_id(its_submodules, cout);
        //*/
        //------------------------------------ cleaning up submodules --------------------------
        set<int> a;
        for (UI i = 0; i < its_submodules.size(); i++) {
            deque<int> grbe;
            sub_graph_module.CUP_check(its_submodules[i], grbe);
            deque_to_set_app(grbe, a);
            //cout<<i<<" cleaned_up: "<<grbe.size()<<" "<<a.size()<<endl;
            if (a.size() > paras.coverage_percentage_fusion_or_submodules * A.size())
                return false;
        }
        //sub_graph_module.draw("sub");
        //cherr();
        return true;
    }   //******************  sub_graph_module stuff   ******************
}

bool oslom_net_global::fusion_with_empty_A(int_matrix &its_submodules, DI &A, double &bs) {
    /*
        its_submodules are the modules to check. the question is if to take its_submodules or the union of them
        the function returns true if it's the union, grc1 is the union cleaned and bs the score
     */
    DI group;
    from_int_matrix_and_deque_to_deque(its_submodules, A, group);
    //cout<<"trying a module of "<<group.size()<<" nodes"<<endl;
    bs = CUP_check(group, A);
    if (A.size() <= paras.coverage_percentage_fusion_left * group.size()) {
        A.clear();
        bs = 1;
        return false;
    }
    bool fus = fusion_module_its_subs(A, its_submodules);
    if (fus == false)
        return false;
    else
        return true;
}

void oslom_net_global::check_existing_unions(module_collection &mall) {
    /* this function is to check unions of existing modules*/
    /* sorting from the biggest to the smallest module */
    /*cout<<"before check_existing_unions"<<endl;
    print_modules(false, cout, mall);*/
    deque<int> sm;
    mall.sort_modules(sm);
    /*cout<<"sm"<<endl;
    prints(sm);*/
    deque<bool> still_good;
    for (UI i = 0; i < sm.size(); i++)
        still_good.push_back(true);
    set<int> modules_to_erase;
    for (UI i = 0; i < sm.size(); i++) {
        /* for each module I check if it's better to take it or its submodules */
        DI smaller;
        mall.almost_equal(sm[i], smaller);
        int_matrix its_submodules;
        for (UI j = 0; j < smaller.size(); j++)
            if (still_good[smaller[j]])
                its_submodules.push_back(mall.modules[smaller[j]]);
        /*cout<<"************************** module to check "<<sm[i]<<" size: "<<mall.modules[sm[i]].size()<<endl;
        print_id(mall.modules[sm[i]], cout);
        cout<<"its_submodules"<<endl;
        print_id(its_submodules, cout);*/
        if (fusion_module_its_subs(mall.modules[sm[i]], its_submodules)) {
            deque_to_set_app(smaller, modules_to_erase);
            for (UI j = 0; j < smaller.size(); j++)
                still_good[smaller[j]] = false;
        } else {
            modules_to_erase.insert(sm[i]);
            still_good[sm[i]] = false;
        }
    }
    for (set<int>::iterator its = modules_to_erase.begin(); its != modules_to_erase.end(); its++)
        mall.erase(*its);
    mall.compact();
    /*cout<<"after check_existing_unions --------------------------------------------------------"<<endl;
    print_modules(false, cout, mall);*/
}

bool oslom_net_global::check_fusion_with_gather(module_collection &mall) {
    /*	this function is used to check if we would like unions of modules
        returns true if it merges something	 */
    cout << "check unions of modules using community network" << endl << endl;
    paras.print_flag_subgraph = true;
    mall.fill_gaps();
    map<int, map<int, pair<int, double> > >
            neigh_weight_s;        // this maps the module id into the neighbor module ids and weights
    set_upper_network(neigh_weight_s, mall);
    if (neigh_weight_s.size() == 0)
        return false;
    bool wgather = true;
    bool real_paras_weighted = paras.weighted;
    if (wgather) {
        for (map<int, map<int, pair<int, double> > >::iterator itm = neigh_weight_s.begin(); itm !=
                                                                                             neigh_weight_s.end();
             itm++) {
            for (map<int, pair<int, double> >::iterator itm2 = itm->second.begin(); itm2 != itm->second.end();
                 itm2++) {
                itm2->second.first = 1;
            }
        }
        paras.weighted = true;
    }
    oslom_net_global community_net(neigh_weight_s);
    int_matrix M_raw;        /* M_raw contains the module_ids */
    community_net.collect_raw_groups_once(M_raw);
    paras.weighted = real_paras_weighted;
    bool something = false;
    int_matrix module_to_insert;
    DD bs_to_insert;
    int fused_modules = 0;
    cout << "possible fusions to check: " << M_raw.size() << endl;
    for (UI i = 0; i < M_raw.size(); i++)
        if (M_raw[i].size() > 1) {
            int_matrix ten;
            for (UI j = 0; j < M_raw[i].size(); j++)
                ten.push_back(mall.modules[M_raw[i][j]]);
            //cout<<"trying fusion # "<<i<<" "<<ten.size()<<" modules to merge"<<endl;
            DI grc1;
            double bs;
            if (fusion_with_empty_A(ten, grc1, bs)) {
                something = true;
                module_to_insert.push_back(grc1);
                ++fused_modules;
                bs_to_insert.push_back(bs);
            }
            if (i % 100 == 0)
                cout << "checked " << i << " unions. Fused: " << fused_modules << endl;
        }
    for (UI i = 0; i < module_to_insert.size(); i++)
        mall.insert(module_to_insert[i], bs_to_insert[i]);
    mall.compute_inclusions();
    return something;
}

int oslom_net_global::check_unions_and_overlap(module_collection &mall, bool only_similar) {
    mall.put_gaps();
    if (mall.effective_groups() == 0)
        return 0;
    cout << "checking similar modules" << endl << endl;
    check_existing_unions(mall);
    if (only_similar == false) {
        if (check_fusion_with_gather(mall))
            check_fusion_with_gather(mall);
    }
    cout << "checking highly intersecting modules" << endl << endl;
    check_intersection(mall);
    mall.compute_inclusions();
    return 0;
}
