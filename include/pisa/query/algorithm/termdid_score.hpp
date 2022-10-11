//
// Created by Hazel on 2022/9/27.
//

#ifndef PISA_TERMDID_SCORE_H
#define PISA_TERMDID_SCORE_H

//template <typename IndexType, typename WandType>
//void termdid_score(
//    const std::string& index_filename,
//    const std::string& wand_data_filename,
//    std::string const& type,
//    std::string const& documents_filename,
//    ScorerParams const& scorer_params,
//    App<arg::Index,
//        arg::WandData<arg::WandMode::Required>,
//        arg::Query<arg::QueryMode::Ranked>,
//        arg::Algorithm,
//        arg::Scorer,
//        arg::Thresholds,
//        arg::Threads> &
//        app);

namespace pisa {

struct termdid_search {
    template <typename CursorRange>
    double operator()(CursorRange&& cursors, uint64_t max_docid, uint64_t target_docid) {
        using Cursor = typename std::decay_t<CursorRange>::value_type;
        if (cursors.empty()) {
            return -1;
        }
        cursors[0].next_geq(target_docid);
        if (cursors[0].docid() != target_docid) {
            return 0;
        }
        return cursors[0].score();
    }
};

struct termdidlist_search {
    template <typename CursorRange>
    std::vector<double> operator()(CursorRange&& cursors, uint64_t max_docid, std::vector<uint64_t> vec_target_did) {
        using Cursor = typename std::decay_t<CursorRange>::value_type;
        if (cursors.empty()) {
            vector<double> vect(1, -1);
            return vect;
        }
        clock_t start, end;
        start = clock();
        std::vector<double> vec_score;
        for (auto target_did: vec_target_did) {
            cursors[0].next_geq(target_did);
            // std::cout << "When target_did is " << target_did << ". nextgeq returns " << cursors[0].docid() << '\n';
            if (cursors[0].docid() != target_did) {
                vec_score.push_back(0);
            }
            else {
                vec_score.push_back(cursors[0].score());
            }
        }
        end = clock();
        double running_time = double(end - start) / double(CLOCKS_PER_SEC);
        std::cout << "Runting time of read a posting list is " << running_time << "s\n";
        return vec_score;
    }
};

}  // namespace pisa

#endif  // PISA_TERMDID_SCORE_H
