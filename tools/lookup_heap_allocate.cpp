#include "iostream"
#include "vector"
#include "tuple"
#include "util/hit_ratio_utils.hpp"
// #include <../include/utils.h>

//class HitRatioHeap {
//  public:
//    HitRatioHeap() {
//
//    }
//  private:
//    std::vector
//
//};

int main(int argc, const char** argv) {
    std::vector<std::string> lst_query_term = {"armadillo", "cat", "dog", "mouse", "okapi"};
    std::vector<int> lst_type = {1, 2, 3, 4};
    std::tuple<std::vector<std::string>,
               std::vector<std::string>,
               std::vector<std::string>,
               std::vector<std::string>> tuple_gram_result = get_grams(lst_query_term, lst_type);

    std::vector<std::string>lst_single, lst_duplet, lst_triplet, lst_quadruplet;
    lst_single = std::get<0>(tuple_gram_result);
    lst_duplet = std::get<1>(tuple_gram_result);
    lst_triplet = std::get<2>(tuple_gram_result);
    lst_quadruplet = std::get<3>(tuple_gram_result);
    //    for (auto v: lst_single)
    //        std::cout << v << '\n';
    //    for (auto v: lst_duplet)
    //        std::cout << v << '\n';
    //    for (auto v: lst_triplet)
    //        std::cout << v << '\n';
    //    for (auto v: lst_quadruplet)
    //        std::cout << v << '\n';

    std::cout << "Starting to test hit_ratio computation...\n";
    HitRatioHeap tmp_obj;

    std::string query_name = "1017_blond blue eye girl hair with";
    std::vector lst_budget = {500, 1000, 2000, 3000, 4000, 5000};
    std::vector lst_top_k = {10, 100, 1000};
    std::vector lst_type2 = {1, 2, 3, 4};
    std::string index_path = "/home/jg6226/data/Hit_Ratio_Project/First_Layer_Index/20K/Add_Both_Pair";
    std::string query_set_path = "/home/jg6226/data/Hit_Ratio_Project/Split_Test_Output";
    std::unordered_map<int, std::vector<double>> result = tmp_obj.hit_ratio_heap(query_name, lst_budget, lst_top_k, lst_type2, index_path, query_set_path);

    //    std::unordered_map<int, std::vector<double>> result = tmp_obj.hit_ratio_heap("1017_blond blue eye girl hair with", {500, 1000, 2000, 3000, 4000, 5000},
    //                                                                                 {10, 100, 1000}, {1, 2, 3, 4}, "/home/jg6226/data/Hit_Ratio_Project/First_Layer_Index/20K/Add_Both_Pair",
    //                                                                                 "/home/jg6226/data/Hit_Ratio_Project/Split_Test_Output");

    for (auto top_k: {10, 100, 1000})
    {
        std::cout << top_k << '\n';
        for (int i = 0; i < result[top_k].size(); ++i)
        {
            std::cout << result[top_k].at(i) << " ";
        }
        std::cout << '\n';
    }

    tmp_obj.compute_heap_accuracy(query_set_path, lst_budget, lst_top_k, lst_type2, index_path);


    //    for (auto element: result)
    //    {
    //        std::cout << element.first << '\n';
    //        for (int i = 0; i < element.second.size(); ++i)
    //        {
    //            std::cout << element.second.at(i) << " ";
    //        }
    //        std::cout << '\n';
    //    }
}