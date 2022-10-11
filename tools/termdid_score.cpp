//
// Created by Hazel on 2022/9/27.
//
#include <iostream>
#include <optional>
#include <thread>

#include <CLI/CLI.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
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

template <typename IndexType, typename WandType>
void termdid_score(
    const std::string& index_filename,
    const std::string& wand_data_filename,
    const Query& single_query,
    std::string const& type,
    std::string const& documents_filename,
    ScorerParams const& scorer_params,
    uint64_t target_did)
{
    IndexType index(MemorySource::mapped_file(index_filename));
    WandType const wdata(MemorySource::mapped_file(wand_data_filename));

    auto scorer = scorer::from_params(scorer_params, wdata);
    std::function<std::vector<typename topk_queue::entry_type>(Query)> query_fun;

    termdid_search td_s;
    double lookup_score = td_s(make_max_scored_cursors(index, wdata, *scorer, single_query) , index.num_docs(), target_did);

    auto source = std::make_shared<mio::mmap_source>(documents_filename.c_str());
    auto docmap = Payload_Vector<>::from(*source);

    std::cout << "The term " << single_query.terms[0] << " in document " << docmap[target_did] << " is " << lookup_score << '\n';
}

using wand_raw_index = wand_data<wand_data_raw>;
using wand_uniform_index = wand_data<wand_data_compressed<>>;
using wand_uniform_index_quantized = wand_data<wand_data_compressed<PayloadType::Quantized>>;

int main(int argc, const char** argv) {
    spdlog::set_default_logger(spdlog::stderr_color_mt("default"));

    std::string documents_file;
    int docid;

    App<arg::Index,
        arg::WandData<arg::WandMode::Required>,
        arg::Query<arg::QueryMode::Ranked>,
        arg::Algorithm,
        arg::Scorer,
        arg::Thresholds,
        arg::Threads>
        app{"Retrieves query results in TREC format."};
    app.add_option("--documents", documents_file, "Document lexicon")->required();
    app.add_option("--docid", docid, "Document ID to lookup, not Trec ID")->required();

    CLI11_PARSE(app, argc, argv);

    //    auto params = std::make_tuple(
    //        app.index_filename(),
    //        app.wand_data_path(),
    //        app.lookup(),
    //        app.index_encoding(),
    //        documents_file,
    //        app.scorer_params(),
    //        docid);

    termdid_score<block_simdbp_index, wand_raw_index>(app.index_filename(), app.wand_data_path(), app.lookup(), app.index_encoding(), \
                                                      documents_file, app.scorer_params(), docid);

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