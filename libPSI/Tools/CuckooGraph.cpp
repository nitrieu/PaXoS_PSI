#include "CuckooGraph.h"
#include "cryptoTools/Crypto/sha1.h"
#include "cryptoTools/Crypto/PRNG.h"
#include <random>
#include "cryptoTools/Common/Log.h"
#include <numeric>

namespace osuCrypto
{
	void MyVisitor::init(u64 inputSize, u64 numHashs, u64 numBins,  u64 sigma)
	{
		mInputSize = inputSize;
		mNumHashs = numHashs;
		mNumBins = numBins;
		mSigma = sigma;
	}

	void MyVisitor::buidingGraph(span<block> inputs)
	{
		PRNG prng(ZeroBlock);
		mAesHasher.setKey(OneBlock);
	
		hashes1.resize(inputs.size());
		hashes2.resize(inputs.size());
		//strHashesIncr.resize(inputs.size());
		
		std::map<string, int> groupHash; //TODO: edgeId mapping // std::map<EdgeID, int> mEdgeIdxMap2;
		for (int idxItem = 0; idxItem < inputs.size(); idxItem++)
		{
			block cipher = mAesHasher.ecbEncBlock(inputs[idxItem]);
			u64 hh1 = _mm_extract_epi64(cipher, 0);
			u64 hh2 = _mm_extract_epi64(cipher, 1);

			hashes1[idxItem] = hh1 % mNumBins; //1st 64 bits for finding bin location
			hashes2[idxItem] = hh2 % mNumBins; //2nd 64 bits for finding alter bin location

			//std::cout << "hashes: " << hashes1[idxItem] << " === " << hashes2[idxItem] << "  == " << mNumBins << std::endl;

			auto key = concateIntsIncr(hashes1[idxItem] , hashes2[idxItem] , mNumBins); //h1||h2

			auto  p = groupHash.find(key);

			if (p == groupHash.end())
			{
				groupHash.emplace(pair<string, int>(key, idxItem));
				//std::cout << "groupHash: " << strHashesIncr[idxItem] << " === " << idxItem << std::endl;
			}
			else
			{
				//mStrBadItems.insert(strHashesIncr[idxItem]);
				//mIdxBadItems.insert(idxItem); // for test
				mBadEdges.insert({ hashes1[idxItem],hashes2[idxItem] });
				std::cout << "mIdxBadItems============ " << idxItem << std::endl;

			}
		}

		ccGraph g(groupHash.size()); //init vertext 0,,m

		for (auto it = groupHash.begin(); it != groupHash.end(); ++it)
		{
			u64 idxItem = it->second;
			auto e = boost::add_edge(hashes1[idxItem], hashes2[idxItem], g);
			mEdgeIdxMap_new.insert(pair<set<u64>, int>({ hashes1[idxItem],hashes2[idxItem] }, idxItem));
			//std::cout << e.first << "  " << idxItem <<"\n";
		}
		//std::cout << mBadEdges.size() << " mBadEdgesmBadEdges.size()\t";


		mCuckooGraph = g;
	}
}
