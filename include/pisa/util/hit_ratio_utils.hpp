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
        tmp.push_back(q_terms.at(i));
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
struct obj_posting
{
    std::string did;
    double impact_score;
    std::ifstream * file_reader;
    obj_posting(std::string did, double impact_score, std::ifstream * file_reader)
        : did(did), impact_score(impact_score), file_reader(file_reader)
    {}
    //    obj_posting(std::string did, double impact_score, std::ifstream * file_reader) {
    //        this->did = did;
    //        this->impact_score = impact_score;
    //        this->file_reader = file_reader;
    //    }
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

    // std::cout << "v1 size: " << v1.size() << ". v2 size: " << v2.size() << '\n';

    std::sort(v1.begin(), v1.end());
    std::sort(v2.begin(), v2.end());

    std::set_intersection(v1.begin(),v1.end(),
                          v2.begin(),v2.end(),
                          back_inserter(v3));

    //    std::set_intersection(v1.begin(),v1.end(),
    //                          v2.begin(),v2.end(),
    //                          back_inserter(v3));

    std::sort(v3.begin(), v3.end());
    // std::cout << "v3 size: " << v3.size() << '\n';
    return v3;
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
        std::vector<std::ifstream *> f_grams;
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
                std::ifstream * tmp_file_reader = new std::ifstream(index_path + "/single/" + filename);
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
                std::ifstream * tmp_file_reader = new std::ifstream(index_path + "/duplet/" + filename);
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
                std::ifstream * tmp_file_reader = new std::ifstream(index_path + "/triplet/" + filename);
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
                std::ifstream * tmp_file_reader = new std::ifstream(index_path + "/quadruplet/" + filename);
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
                if (lst_real.size() == 0)
                {
                    dict_query_acc[top_k].emplace_back(NULL);
                }
                else
                {
                    // std::cout << "For top_k = " << top_k << ": " << lst_intersection.size() << " " << lst_real.size() << " " << "acc is " << double(lst_intersection.size()) / double(lst_real.size()) << '\n';
                    dict_query_acc[top_k].emplace_back(double(lst_intersection.size()) / double(lst_real.size()));
                }
            }

        }
        return dict_query_acc;
    }
  private:
    std::vector<int> lst_budget = {};
    std::vector<std::string> lst_query_name = {};
};