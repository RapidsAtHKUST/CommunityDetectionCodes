//
// Created by cheyulin on 2/26/17.
//
#include "cast.h"

bool cast_string_to_double(string &b, double &h) {
// set h= the number written in b[];
// return false if there is an error
    h = 0;
    if (b.size() == 0)
        return false;
    int sign = 1;
    if (b[0] == '-') {
        b[0] = '0';
        sign = -1;
    }
    int digits_before = 0;
    for (int i = 0; i < b.size(); i++)
        if (b[i] != '.')
            digits_before++;
        else
            break;
    int j = 0;
    while (j != digits_before) {
        int number = (int(b[j]) - 48);
        h += number * pow(10., digits_before - j - 1);
        if (number < 0 || number > 9)
            return false;
        j++;
    }
    j = digits_before + 1;
    while (j < b.size()) {
        int number = (int(b[j]) - 48);
        h += number * pow(10, digits_before - j);
        if (number < 0 || number > 9)
            return false;
        j++;
    }
    h = sign * h;
    return true;
}

int cast_int(double u) {
    int a = int(u);
    if (u - a > 0.5)
        a++;
    return a;
}

int cast_string_to_char(string &file_name, char *b) {
    for (int i = 0; i < file_name.size(); i++)
        b[i] = file_name[i];
    b[file_name.size()] = '\0';
    return 0;
}

bool cast_string_to_doubles(string &b, deque<double> &v) {
    v.clear();
    string s1;
    for (int i = 0; i < int(b.size()); i++)
        if (int(b.size()) > 0 && b[0] != '#') {
            if (b[i] != '1' && b[i] != '2' && b[i] != '3' && b[i] != '4' && b[i] != '5' && b[i] != '6' && b[i] != '7' &&
                b[i] != '8' && b[i] != '9' && b[i] != '0' && b[i] != '.' && b[i] != 'e' && b[i] != '+' && b[i] != '-' &&
                b[i] != 'E') {
                double num;
                if (cast_string_to_double(s1, num))
                    v.push_back(num);
                s1.clear();
            } else
                s1.push_back(b[i]);
            if (i == int(b.size()) - 1) {
                double num;
                if (cast_string_to_double(s1, num))
                    v.push_back(num);
                s1.clear();
            }
        }
    return true;
}

bool cast_string_to_doubles(string &b, deque<int> &v) {
    v.clear();
    deque<double> d;
    cast_string_to_doubles(b, d);
    for (int i = 0; i < int(d.size()); i++)
        v.push_back(cast_int(d[i]));
    return true;
}

bool separate_strings(string &b, deque<string> &v) {
    v.clear();
    string s1;
    for (int i = 0; i < int(b.size()); i++) {
        if (b[i] == ' ' || b[i] == '\t' || b[i] == '\n' || b[i] == ',') {
            if (s1.size() > 0)
                v.push_back(s1);
            s1.clear();
        } else
            s1.push_back(b[i]);
        if (i == int(b.size()) - 1) {
            if (s1.size() > 0)
                v.push_back(s1);
            s1.clear();
        }
    }
    return true;
}

double approx(double a, int digits) {
    digits--;
    bool neg = false;
    if (a < 0) {
        neg = true;
        a = -a;
    }
    //cout<<a<<endl;
    int tpow = 0;
    while (a < pow(10, digits)) {
        tpow++;
        a *= 10;
    }
    while (a > pow(10, digits + 1)) {
        tpow--;
        a /= 10;
    }
    if (neg == false)
        return cast_int(a) / pow(10, tpow);
    else
        return -cast_int(a) / pow(10, tpow);
}

double cast_string_to_double(string &b) {
    double h;
    cast_string_to_double(b, h);
    return h;
}
