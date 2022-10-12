// #pragma once
//
//template <typename item>
//void makeCombiUtil(std::vector<std::string >& ans,
//                   std::vector<item>& tmp, std::vector<item> & q_terms, int n, int left, int k);
//
//template <typename item>
//std::vector<std::string > makeCombi(std::vector<item> & q_terms, int k);
using QueryStr = std::vector<std::string>;
using SubStructure = std::tuple<QueryStr, QueryStr, QueryStr, QueryStr>;

#include "algorithm"
#include "iostream"
#include "fstream"
#include "tuple"
#include "vector"
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include <filesystem>
#include <unordered_map>
#include <queue>
#include <boost/algorithm/string.hpp>

#include <dirent.h>
#include <typeinfo>
#include <range/v3/all.hpp>

// function for print time
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <numeric>
// #include "utils.hpp"

template <typename item>
void makeCombiUtil(std::vector<std::string >& ans,
                   std::vector<item>& tmp, std::vector<item> & q_terms, int n, int left, int k)
{
    // Pushing this vector to a vector of vector
    if (k == 0) {
        std::string joinedString = boost::algorithm::join(tmp, " ");
        ans.push_back(joinedString);
        return;
    }

    // i iterates from left to n. First time
    // left will be 1
    for (int i = left; i < n; ++i)
    {
        tmp.push_back(q_terms[i]);
        makeCombiUtil<item>(ans, tmp, q_terms, n, i + 1, k - 1);

        // Popping out last inserted element
        // from the vector
        tmp.pop_back();
    }
}

// Prints all combinations of size k of numbers
// from 1 to n.
template <typename item>
std::vector<std::string > makeCombi(std::vector<item> & q_terms, int k)
{
    int n = q_terms.size();
    std::vector<std::string > ans;
    std::vector<item> tmp;
    makeCombiUtil<item> (ans, tmp, q_terms, n, 0, k);
    return ans;
}

SubStructure get_grams(std::vector<std::string> & q_terms, std::vector<int> & lst_type)
{
    std::vector<std::string>lst_single, lst_duplet, lst_triplet, lst_quadruplet;

    if (std::find(lst_type.begin(), lst_type.end(), 1) != lst_type.end()) {
        lst_single = makeCombi<std::string>(q_terms, 1);
    }

    if (std::find(lst_type.begin(), lst_type.end(), 2) != lst_type.end()) {
        lst_duplet = makeCombi<std::string>(q_terms, 2);
    }

    if (std::find(lst_type.begin(), lst_type.end(), 3) != lst_type.end()) {
        lst_triplet = makeCombi<std::string>(q_terms, 3);
    }

    if (std::find(lst_type.begin(), lst_type.end(), 4) != lst_type.end()) {
        lst_quadruplet = makeCombi<std::string>(q_terms, 4);
    }

    return std::make_tuple(lst_single, lst_duplet, lst_triplet, lst_quadruplet);
}

//static struct obj_posting
using instream_ptr = std::shared_ptr<std::ifstream>;
struct obj_posting
{
    std::string did;
    double impact_score;
    instream_ptr file_reader;
    obj_posting(std::string did, double impact_score, instream_ptr file_reader)
        : did(did), impact_score(impact_score), file_reader(file_reader)
    {}
    bool operator<(const obj_posting & rhs) const
    {
        return this->impact_score < rhs.impact_score;
    }

    bool operator>(const obj_posting & rhs) const
    {
        return this->impact_score > rhs.impact_score;
    }

    bool operator==(const obj_posting & rhs) const
    {
        return this->impact_score == rhs.impact_score;
    }
};

struct compare
{
    bool operator()(const obj_posting & a, const obj_posting & b)
    {
        return a.impact_score > b.impact_score;
    }
};

std::vector<std::string> intersection(std::vector<std::string> & v1,
                                      std::vector<std::string> & v2){
    std::vector<std::string> v3;
    std::sort(v1.begin(), v1.end());
    std::sort(v2.begin(), v2.end());

    std::set_intersection(v1.begin(),v1.end(),
                          v2.begin(),v2.end(),
                          back_inserter(v3));
    std::sort(v3.begin(), v3.end());
    return v3;
}

template <typename v_type>
void print(std::vector<v_type> & v)
{
    std::cout << "[";
    for (auto i: v)
    {
        std::cout << " " << i;
    }
    std::cout << "]\n";
}

class HitRatioHeap {
  public:
    typedef std::unordered_map<int, std::vector<double>> acc_dict;
    acc_dict hit_ratio_heap(
        const std::string & query_name,
        const std::vector<int> & lst_budget,
        const std::vector<int> & lst_top_k,
        std::vector<int> lst_type,
        const std::string & index_path,
        const std::string & query_out_path)
    {
        int max_budget = *std::max_element(lst_budget.begin(), lst_budget.end());
        std::vector<std::string> tmp_string;
        std::vector<std::string> q_terms;
        boost::split(tmp_string, query_name, boost::is_any_of("_"));
        boost::split(q_terms, tmp_string.back(), boost::is_any_of(" "));
        SubStructure sub_structure = get_grams(q_terms, lst_type);
        std::vector<instream_ptr> f_grams;
        std::vector<std::string> f_names;
        int cnt_valid_gram = 0;
        for (auto & i : std::get<0>(sub_structure))
        {
            std::string filename = "SINGLE_" + i;
            if(std::filesystem::exists(index_path + "/single/" + filename))
            {
                f_names.emplace_back(filename);
                ++cnt_valid_gram;
                // std::ifstream tmp_file_reader(index_path + "/single/" + filename);
                // f_grams.emplace_back(&tmp_file_reader);
                instream_ptr tmp_file_reader(new std::ifstream(index_path + "/single/" + filename));
                f_grams.emplace_back(tmp_file_reader);
                // f_grams.emplace_back(&std::ifstream(index_path + "/single/" + filename));
            }
        }

        for (auto & i : std::get<1>(sub_structure))
        {
            std::string filename = "DUPLET_" + i;
            if(std::filesystem::exists(index_path + "/duplet/" + filename))
            {
                f_names.emplace_back(filename);
                ++cnt_valid_gram;
                // std::ifstream tmp_file_reader(index_path + "/duplet/" + filename);
                // f_grams.emplace_back(&tmp_file_reader);
                instream_ptr tmp_file_reader(new std::ifstream(index_path + "/duplet/" + filename));
                f_grams.emplace_back(tmp_file_reader);
                // f_grams.emplace_back(&std::ifstream(index_path + "/duplet/" + filename));
            }
        }

        for (auto & i : std::get<2>(sub_structure))
        {
            std::string filename = "TRIPLET_" + i;
            if(std::filesystem::exists(index_path + "/triplet/" + filename))
            {
                f_names.emplace_back(filename);
                ++cnt_valid_gram;
                // std::ifstream tmp_file_reader(index_path + "/triplet/" + filename);
                // f_grams.emplace_back(&tmp_file_reader);
                instream_ptr tmp_file_reader(new std::ifstream(index_path + "/triplet/" + filename));
                f_grams.emplace_back(tmp_file_reader);
                // f_grams.emplace_back(&std::ifstream(index_path + "/triplet/" + filename));
            }
        }

        for (auto & i : std::get<3>(sub_structure))
        {
            std::string filename = "QUADRUPLET_" + i;
            if(std::filesystem::exists(index_path + "/quadruplet/" + filename))
            {
                f_names.emplace_back(filename);
                ++cnt_valid_gram;
                // std::ifstream tmp_file_reader(index_path + "/quadruplet/" + filename);
                // f_grams.emplace_back(&tmp_file_reader);
                instream_ptr tmp_file_reader(new std::ifstream(index_path + "/quadruplet/" + filename));
                f_grams.emplace_back(tmp_file_reader);
                // f_grams.emplace_back(&std::ifstream(index_path + "/quadruplet/" + filename));
            }
        }

        // std::priority_queue<obj_posting, std::vector<obj_posting>, compare> hp_posting;
        std::priority_queue<obj_posting> hp_posting;

        for(auto i : f_grams)
        {
            std::string line;
            std::getline(*i, line);
            // std::cout << line.empty() << '\n';
            // std::cout << line << '\n';
            if (!line.empty())
            {
                std::vector<std::string> line_list;
                boost::algorithm::trim(line);
                boost::split(line_list, line, boost::is_any_of(" "));
                double score_raw = std::stod(line_list[2]);
                // std::cout << "Score is " << line_list[2] << " " << score_raw << '\n';
                hp_posting.push(obj_posting(line_list[1], score_raw, i));
            }
        }

        std::vector<std::string> lst_pred_did;
        while (!hp_posting.empty() && max_budget > 0)
        {
            obj_posting current_obj = hp_posting.top();
            hp_posting.pop();
            // std::cout << "Debugging:" << current_obj.did << '\n';
            lst_pred_did.emplace_back(current_obj.did);
            --max_budget;

            std::string line;
            std::getline(*current_obj.file_reader, line);
            if (!line.empty())
            {
                std::vector<std::string> line_list;
                boost::algorithm::trim(line);
                boost::split(line_list, line, boost::is_any_of(" "));
                double score_raw = std::stod(line_list[2]);
                hp_posting.push(obj_posting(line_list[1], score_raw, current_obj.file_reader));
            }
        }

        for(auto i : f_grams)
        {
            i->close();
        }

        int max_top_k = *std::max_element(lst_top_k.begin(), lst_top_k.end());
        std::vector<std::string> lst_real_did;
        std::ifstream f_real_output(query_out_path + '/' + query_name);
        int cnt_top = 0;
        std::string line;
        // std::getline(f_real_output, line);
        while (std::getline(f_real_output, line) && cnt_top < max_top_k)
        {
            std::vector<std::string> line_list;
            boost::algorithm::trim(line);
            boost::split(line_list, line, boost::is_any_of(" "));
            lst_real_did.emplace_back(line_list[1]);
            cnt_top += 1;
        }
        f_real_output.close();

        // in order to compute MAP and MRR, use a temp dic to see whether a did is in top k;
        // std::unordered_map<int, std::unordered_map<>> dict_top_k;
        std::unordered_map<int, std::vector<double>> dict_query_acc;
//#define TEST_SLICE
#ifdef TEST_SLICE
        for (auto top_k: lst_top_k)
        {
            dict_query_acc[top_k] = {};
            int real_len_to_slice = std::min(top_k, int(lst_real_did.size()));
            auto lst_real = lst_real_did | ranges::views::slice(0, real_len_to_slice);
//            std::vector<std::string> bbb;
//            for(auto & i : lst_real)
//            {
//                bbb.push_back(i);
//            }
            for (auto temp_budget: lst_budget)
            {
                int pred_len_to_slice = std::min(temp_budget, int(lst_pred_did.size()));
                auto lst_pred = lst_pred_did | ranges::views::slice(0, pred_len_to_slice);
                ranges::sort(lst_pred);
                ranges::sort(lst_real);
                auto lst_intersection = ranges::views::set_intersection(lst_real, lst_pred);
                std::vector<std::string> bbb;
                for(auto & i : lst_real)
                {
                    bbb.push_back(i);
                }
                int a = 0;
                ranges::for_each(lst_intersection, [&a](std::string & s){a++;});
                int b = lst_real.size();
                if (lst_real.empty())
                {
                    // dict_query_acc[top_k].emplace_back(NULL);
                    dict_query_acc[top_k].emplace_back(-1.0);
                }
                else
                {
                    // std::cout << "For top_k = " << top_k << ": " << lst_intersection.size() << " " << lst_real.size() << " " << "acc is " << double(lst_intersection.size()) / double(lst_real.size()) << '\n';
                    dict_query_acc[top_k].emplace_back(double(a) / double(lst_real.size()));
                }
            }

        }
#endif
#ifndef TEST_SLICE
        for (auto top_k: lst_top_k)
        {
            dict_query_acc[top_k] = {};
            int real_len_to_slice = std::min(top_k, int(lst_real_did.size()));
            std::vector<std::string> lst_real = std::vector<std::string>(lst_real_did.begin(), lst_real_did.begin() + real_len_to_slice);
            // std::cout << "Top k size: " << lst_real.size() << '\n';
            // std::vector<std::string> lst_real = std::vector<std::string>(lst_real_did.begin(), lst_real_did.begin() + real_len_to_slice);
            for (auto temp_budget: lst_budget)
            {
                int pred_len_to_slice = std::min(temp_budget, int(lst_pred_did.size()));
                std::vector<std::string> lst_pred = std::vector<std::string>(lst_pred_did.begin(), lst_pred_did.begin() + pred_len_to_slice);
                // std::vector<std::string> lst_pred = std::vector<std::string>(lst_pred_did.begin(), lst_pred_did.begin() + pred_len_to_slice);
                std::vector<std::string> lst_intersection = intersection(lst_real, lst_pred);
                int c = lst_intersection.size();
                int b = lst_real.size();
                if (lst_real.size() == 0)
                {
                    // dict_query_acc[top_k].emplace_back(NULL);
                    dict_query_acc[top_k].emplace_back(-1.0);
                }
                else
                {
                    // std::cout << "For top_k = " << top_k << ": " << lst_intersection.size() << " " << lst_real.size() << " " << "acc is " << double(lst_intersection.size()) / double(lst_real.size()) << '\n';
                    dict_query_acc[top_k].emplace_back(double(lst_intersection.size()) / double(lst_real.size()));
                }
            }

        }
#endif
        return dict_query_acc;
    }

    void compute_heap_accuracy(const std::string query_set_path, std::vector<int> lst_budget, std::vector<int> lst_top_k, std::vector<int> lst_type, std::string index_path)
    {
        this->lst_budget = lst_budget;
        std::vector<std::string> lst_files;

        DIR *dpdf;
        struct dirent *epdf;
        char* qsp = const_cast<char*>(query_set_path.c_str());
        dpdf = opendir(qsp);
        if (dpdf != NULL) {
            epdf = readdir(dpdf);
            while (epdf != nullptr) {
                lst_files.emplace_back(std::string(epdf->d_name));
                epdf = readdir(dpdf);
            }
        }

        std::unordered_map<int, std::vector<double>> dict_avg_acc = {};
        std::unordered_map<int, std::vector<std::vector<double>>> dict_aux_lst = {};

        for (auto top_k: lst_top_k)
        {
            dict_avg_acc[top_k] = {};
            std::vector<std::vector<double>> tmp_vec = {};
            dict_aux_lst[top_k] = tmp_vec;
        }

        for (auto top_k: lst_top_k)
        {
            for (int i = 0; i < lst_budget.size(); ++i)
            {
                std::vector<double> tmp_vec = {};
                dict_aux_lst[top_k].emplace_back(tmp_vec);
            }
        }

        int cnt_file = 0;
        for (auto test_file: lst_files)
        {
            std::unordered_map<int, std::vector<double>> dict_query_acc = this->hit_ratio_heap(test_file, lst_budget, lst_top_k, lst_type, index_path, query_set_path);
            for (std::pair<int, std::vector<double>> element : dict_query_acc)
            {
                int top_k = element.first;
                std::vector<double> lst_query_acc = element.second;
                for (int i = 0; i < lst_budget.size(); ++i)
                {
                    if (lst_query_acc[i] != -1)
                    {
                        dict_aux_lst[top_k][i].emplace_back(lst_query_acc[i]);
                    }
                }
            }
            cnt_file += 1;
            if (cnt_file % 100 == 0)
            {
                time_t my_time = time(NULL);
                printf("%d files processed, time: %s\n", cnt_file, ctime(&my_time));
            }
        }

        for (std::pair<int, std::vector<std::vector<double>>> element: dict_aux_lst)
        {
            int top_k = element.first;
            std::vector<std::vector<double>> v = element.second;
            for (auto & i : v)
            {
                double sum_of_acc = std::accumulate(i.begin(), i.end(), 0);
                dict_avg_acc[top_k].emplace_back(sum_of_acc / i.size());
            }
        }
        for (auto top_k: lst_top_k)
        {
            std::vector<double> v = dict_avg_acc[top_k];
            std::cout << "When k = " << top_k << '\n';
            print<double>(v);
        }

    }
  private:
    std::vector<int> lst_budget = {};
    std::vector<std::string> lst_query_name = {};
};