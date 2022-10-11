//
// Created by Hazel on 2022/9/27.
//
#include <iostream>
#include <optional>
#include <thread>

#include <CLI/CLI.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <functional>
#include <mappable/mapper.hpp>
#include <mio/mmap.hpp>
#include <range/v3/view/enumerate.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <tbb/global_control.h>
#include <tbb/parallel_for.h>

#include "accumulator/lazy_accumulator.hpp"
#include "app.hpp"
#include "cursor/block_max_scored_cursor.hpp"
#include "cursor/max_scored_cursor.hpp"
#include "cursor/scored_cursor.hpp"
#include "index_types.hpp"
#include "io.hpp"
#include "query/algorithm.hpp"
#include "scorer/scorer.hpp"
#include "util/util.hpp"
#include "wand_data_compressed.hpp"
#include "wand_data_raw.hpp"
#include "boost/algorithm/string/join.hpp"

#include "pisa/query/algorithm/termdid_score.hpp"

using namespace pisa;
using ranges::views::enumerate;

struct term_and_didlist {
    const Query single_query;
    std::vector<uint64_t> vec_target_did;
};

term_and_didlist get_term_and_didlist(std::string &input_line, App<arg::Index,
                                                                   arg::WandData<arg::WandMode::Required>,
                                                                   arg::Query<arg::QueryMode::Ranked>,
                                                                   arg::Algorithm,
                                                                   arg::Scorer,
                                                                   arg::Thresholds,
                                                                   arg::Threads> &
                                                                   app) {
    std::vector<std::string> input_args;
    boost::remove_erase(input_line, '\n');
    // std::cout << "Current lookup: " << input_line << '\n';
    boost::split(input_args, input_line, boost::is_any_of("&"), boost::token_compress_on);

    // now, first arg is the query term and the second arg is the path of did list
    // for (auto item: input_args)
    //     std::cout << item << '\n';


    std::string single_query = input_args[0];

    std::vector<uint64_t> vec_target_did;
    std::ifstream f_target_did(input_args[1]);
    std::string line;

    int tmp_did;
    while (std::getline(f_target_did, line)) {
        boost::remove_erase(line, '\n');
        // std::erase(line.begin(), line.end(), '\n')
        tmp_did = std::stoi(line);
        vec_target_did.push_back(tmp_did);
    }
    f_target_did.close();

    term_and_didlist term_didlist_to_return = {app.lookup(single_query), vec_target_did};

    return term_didlist_to_return;

}

template <typename IndexType, typename WandType>
void termdid_score(
    const std::string& index_filename,
    const std::string& wand_data_filename,
    std::string const& type,
    std::string const& documents_filename,
    ScorerParams const& scorer_params,
    App<arg::Index,
        arg::WandData<arg::WandMode::Required>,
        arg::Query<arg::QueryMode::Ranked>,
        arg::Algorithm,
        arg::Scorer,
        arg::Thresholds,
        arg::Threads> &
        app)
{
    clock_t index_read_start, index_read_end;
    index_read_start = clock();
    IndexType index(MemorySource::mapped_file(index_filename));
    WandType const wdata(MemorySource::mapped_file(wand_data_filename));

    auto scorer = scorer::from_params(scorer_params, wdata);
    std::function<std::vector<typename topk_queue::entry_type>(Query)> query_fun;

    auto source = std::make_shared<mio::mmap_source>(documents_filename.c_str());
    auto docmap = Payload_Vector<>::from(*source);

    index_read_end = clock();
    double running_time = double(index_read_end - index_read_start) / double(CLOCKS_PER_SEC);
    std::cout << "Runting time of reading index " << running_time << "s\n";

    std::string term_and_f_did;
    while (std::cin >> term_and_f_did) {
        termdidlist_search td_s;

        term_and_didlist tmp_struct = get_term_and_didlist(term_and_f_did, app);
        const Query& single_query = tmp_struct.single_query;
        std::vector<uint64_t> & vec_target_did = tmp_struct.vec_target_did;

        std::vector<double> lookup_score = td_s(make_max_scored_cursors(index, wdata, *scorer, single_query) , index.num_docs(), vec_target_did);
        //    std::cout << "Yes\n";
        //    std::cout << "Size of lookup_score:" << lookup_score.size() << '\n';

        int i = 0;
        for (auto target_did: vec_target_did) {
            std::cout << "The term " << single_query.terms[0] << " in document " << docmap[target_did] << " is " << lookup_score[i] << '\n';
            ++i;
        }
    }

}

using wand_raw_index = wand_data<wand_data_raw>;
using wand_uniform_index = wand_data<wand_data_compressed<>>;
using wand_uniform_index_quantized = wand_data<wand_data_compressed<PayloadType::Quantized>>;

int main(int argc, const char** argv) {
    spdlog::set_default_logger(spdlog::stderr_color_mt("default"));

    std::string documents_file;

    App<arg::Index,
        arg::WandData<arg::WandMode::Required>,
        arg::Query<arg::QueryMode::Ranked>,
        arg::Algorithm,
        arg::Scorer,
        arg::Thresholds,
        arg::Threads>
        app{"Retrieves query results in TREC format."};
    app.add_option("--documents", documents_file, "Document lexicon")->required();

    CLI11_PARSE(app, argc, argv);

    //    auto params = std::make_tuple(
    //        app.index_filename(),
    //        app.wand_data_path(),
    //        app.lookup(),
    //        app.index_encoding(),
    //        documents_file,
    //        app.scorer_params(),
    //        docid);

    termdid_score<block_simdbp_index, wand_raw_index>(app.index_filename(), app.wand_data_path(), app.index_encoding(), \
                                                      documents_file, app.scorer_params(), app);

    //#define LOOP_BODY(R, DATA, T) \
//    std::apply(termdid_score<BOOST_PP_CAT(T, _index), wand_raw_index>, params); \
//    BOOST_PP_SEQ_FOR_EACH(LOOP_BODY, _, PISA_INDEX_TYPES);
    //#undef LOOP_BODY

    //    /**/
    //    if (false) {  // NOLINT
    //#define LOOP_BODY(R, DATA, T)                                                                      \
//    }                                                                                              \
//    else if (app.index_encoding() == BOOST_PP_STRINGIZE(T))                                        \
//    {                                                                                              \
//        if (app.is_wand_compressed()) {                                                            \
//            if (quantized) {                                                                       \
//                std::apply(                                                                        \
//                    termdid_score<BOOST_PP_CAT(T, _index), wand_uniform_index_quantized>,       \
//                    params);                                                                       \
//            } else {                                                                               \
//                std::apply(termdid_score<BOOST_PP_CAT(T, _index), wand_uniform_index>, params); \
//            }                                                                                      \
//        } else {                                                                                   \
//            std::apply(termdid_score<BOOST_PP_CAT(T, _index), wand_raw_index>, params);         \
//        }                                                                                          \
//        /**/
    //
    //        BOOST_PP_SEQ_FOR_EACH(LOOP_BODY, _, PISA_INDEX_TYPES);
    //#undef LOOP_BODY
    //    } else {
    //        spdlog::error("Unknown type {}", app.index_encoding());
    //    }


}