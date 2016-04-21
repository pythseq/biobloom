/*
 * BloomMapClassifier.h
 *
 *  Created on: Mar 23, 2016
 *      Author: cjustin
 */

#ifndef BLOOMMAPCLASSIFIER_H_
#define BLOOMMAPCLASSIFIER_H_

#include <string>
#include "boost/shared_ptr.hpp"
#include "Common/Options.h"
#include "Options.h"
#include <google/dense_hash_map>
#include "Common/Dynamicofstream.h"
#include <vector>
#include "DataLayer/FastaReader.h"
#include "BioBloomClassifier.h"
#include "bloomfilter/BloomMapSSBitVec.hpp"
#include "bloomfilter/RollingHashIterator.h"

using namespace std;

class BloomMapClassifier {
public:
	explicit BloomMapClassifier(const string &filterFile);
	void filter(const vector<string> &inputFiles);

	void filterPair(const string &file1, const string &file2);

	virtual ~BloomMapClassifier();
private:
	BloomMapSSBitVec<ID> m_filter;
	vector<string> m_fullIDs;
	google::dense_hash_map<ID, string> m_ids;

	//TODO: REFACTOR WITH BioBloomClassifier
	inline void printSingleToFile(const string &outputFileName,
			const FastqRecord &rec,
			google::dense_hash_map<string, boost::shared_ptr<Dynamicofstream> > &outputFiles) {
		if (opt::outputType == "fa") {
#pragma omp critical(outputFiles)
			{
				(*outputFiles[outputFileName]) << ">" << rec.id << "\n"
						<< rec.seq << "\n";
			}
		} else {
#pragma omp critical(outputFiles)
			{
				(*outputFiles[outputFileName]) << "@" << rec.id << "\n"
						<< rec.seq << "\n+\n" << rec.qual << "\n";
			}
		}
	}

	//TODO: REFACTOR WITH BioBloomClassifier
	inline void printPairToFile(const string &outputFileName,
			const FastqRecord &rec1, const FastqRecord &rec2,
			google::dense_hash_map<string, boost::shared_ptr<Dynamicofstream> > &outputFiles) {
		if (opt::outputType == "fa") {
#pragma omp critical(outputFiles)
			{
				(*outputFiles[outputFileName + "_1"]) << ">" << rec1.id
						<< "\n" << rec1.seq << "\n";
				(*outputFiles[outputFileName + "_2"]) << ">" << rec2.id
						<< "\n" << rec2.seq << "\n";
			}
		} else {
#pragma omp critical(outputFiles)
			{
				(*outputFiles[outputFileName + "_1"]) << "@" << rec1.id << "\n"
						<< rec1.seq << "\n+\n" << rec1.qual << "\n";
				(*outputFiles[outputFileName + "_2"]) << "@" << rec2.id << "\n"
						<< rec2.seq << "\n+\n" << rec2.qual << "\n";
			}
		}
	}

	/*
	 * Returns the number of hits
	 */
	inline unsigned evaluateRead(const FastqRecord &rec,
			google::dense_hash_map<ID, unsigned> &hitCounts) {
		RollingHashIterator itr(rec.seq, m_filter.getKmerSize(),
				m_filter.getSeedValues());
		unsigned nonZeroCount = 0;

		while (itr != itr.end()) {
			ID id = m_filter.atBest(*itr, opt::allowMisses);

			if (id != 0) {
				if (id != opt::COLLI) {
					if (hitCounts.find(id) != hitCounts.end()) {
						++hitCounts[id];
					} else {
						hitCounts[id] = 1;
					}
				}
				++nonZeroCount;
			}
			++itr;
		}
		return nonZeroCount;
	}

	/*
	 * Returns a vector of hits to a specific read
	 */
	vector<ID> convertToHits(
			const google::dense_hash_map<ID, unsigned> &hitCounts,
			unsigned threshold) {
		vector<ID> hits;
		for (google::dense_hash_map<ID, unsigned>::const_iterator i =
				hitCounts.begin(); i != hitCounts.end(); ++i) {
			if (i->second >= threshold) {
				hits.push_back(i->first);
			}
		}
		return hits;
	}

	/*
	 * Returns a vector of hits to a specific read
	 * Both reads much match
	 */
	vector<ID> convertToHitsBoth(
			const google::dense_hash_map<ID, unsigned> &hitCounts1,
			const google::dense_hash_map<ID, unsigned> &hitCounts2,
			unsigned threshold) {
		vector<ID> hits;
		for (google::dense_hash_map<ID, unsigned>::const_iterator i =
				hitCounts1.begin(); i != hitCounts1.end(); ++i) {
			const google::dense_hash_map<ID, unsigned>::const_iterator &j =
					hitCounts2.find(i->first);
			if (j != hitCounts2.end() && i->second >= threshold
					&& j->second >= threshold) {
				hits.push_back(i->first);
			}
		}
		return hits;
	}
};

#endif /* BLOOMMAPCLASSIFIER_H_ */