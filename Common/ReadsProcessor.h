/*
 * ReadsProcessor.h
 *
 *  Created on: Aug 8, 2012
 *      Author: cjustin
 */

#ifndef READSPROCESSOR_H_
#define READSPROCESSOR_H_
#include <string>
#include <stdint.h>

using namespace std;

class ReadsProcessor {
public:
	ReadsProcessor(unsigned windowSize);
	const unsigned char* prepSeq(string const &sequence, size_t position);
	const unsigned char* prepSeq(const char* sequence, size_t position);
	const string getBases(const unsigned char* c); //for debuging purposes
	virtual ~ReadsProcessor();
private:
	//so reallocation does not have to be done
	unsigned char* m_fw;
	unsigned char* m_rv;
	const unsigned m_kmerSize;
	unsigned m_kmerSizeInBytes;
	unsigned m_halfSizeOfKmerInBytes;
	unsigned m_hangingBases; // used if k-mer is indivisible by 4
	unsigned m_hangingBasesExist;
};

#endif /* READSPROCESSOR_H_ */
