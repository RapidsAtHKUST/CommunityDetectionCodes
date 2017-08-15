

int argmax(mapid & topic_distr) {
    
    // returns the key with the biggest value
    
    int argm=-1;
    double maxe= -1e300;
    IT_loop(mapid, itm, topic_distr) {
        if(itm->second>maxe) {
            argm=itm->first;
            maxe=itm->second;
        }
    }
    return argm;    
}


int compute_biggest_key(mapid & m) {
    
    int biggest_key=-2147483647;
    IT_loop(mapid, itm, m) biggest_key=max(itm->first, biggest_key);
    return biggest_key;
    
}



int get_best_of(mapid & a) {
    
    // returns the element with the highest value
    // or -1 if the string is empty 
    
    int m=-1;
    double v=0;
    IT_loop(mapid, itm, a) {
        if(m==-1 or itm->second>v) {
            m=itm->first;
            v=itm->second;
        }
    }
    return m;
}


inline void update_mapid(mapid & m, const int & key, double value) {

    if(m.find(key)==m.end()) {
        m.insert(make_pair(key, 0.));
    }
    m.at(key)+=value;
    if(fabs(m.at(key))<1e-10) {
        m.erase(key);
    } 
}



inline double get_from_mapid(const mapid & m, int key) {
    
    /* this function returns m.at(key) if it exists
     and 0 if it does not */
     
    mapid::const_iterator pos = m.find(key);
    if (pos == m.end()) {
        return 0.;
    } else {
        return pos->second;
    }

}


void check_mapid(mapid & a, mapid & b, string error_message) {
    // this function checks that two mapid
    // are the same
    
    IT_loop(mapid, itm, a) {
        general_assert(b.count(itm->first)>0, error_message);
        assert_floats(b.at(itm->first), itm->second, error_message);
    }
    IT_loop(mapid, itm, b) {
        general_assert(a.count(itm->first)>0, error_message);
        assert_floats(a.at(itm->first), itm->second, error_message);
    }
}


double normalize_mapid(mapid & a) {
    
    double norm=0.;
    IT_loop(mapid, itm, a) norm+=itm->second;
    if(norm<1e-10) {
        a.clear();
    }
    else {
        IT_loop(mapid, itm, a) itm->second/=norm;
    }
    return norm;
}



double sum_mapid(mapid & a) {
    
    double norm=0.;
    IT_loop(mapid, itm, a) norm+=itm->second;
    return norm;
}



void check_distr_equal(mapid & distr1, mapii & distr2) {
    
    mapid distr2norm;
    IT_loop(mapii, itm, distr2) distr2norm[itm->first]=double(itm->second);
    normalize_mapid(distr2norm);
    
    cout<<"check_distr_equal"<<endl;
    IT_loop(mapid, itm, distr1) {
        //cout<<itm->first<<" "<<itm->second<<" "<<distr2norm.at(itm->first)<<endl;
        if(fabs(itm->second-distr2norm.at(itm->first))>1e-5) {
            cerr<<"error in check_distr_equal"<<endl;
            exit(-1);
        }
    }
    cout<<"done"<<endl;
    
}

