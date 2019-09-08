#include "CuckooGraph.h"
#include "cryptoTools/Crypto/sha1.h"
#include "cryptoTools/Crypto/PRNG.h"
#include <random>
#include "cryptoTools/Common/Log.h"
#include <numeric>

namespace osuCrypto
{
	void CuckooGraph::init(u64 inputSize, u64 numHashs, u64 numBins)
	{
		mInputSize = inputSize;
		mNumHashs = numHashs;
		mNumBins = numBins;
		//mEdgeIdxMap(mInputSize);

	}
	void CuckooGraph::buidingGraph(span<block> inputs)
	{
		PRNG prng(ZeroBlock);
		mAesHasher.setKey(prng.get<block>());
	
	//	std::unordered_map<std::string, std::vector<int>> groupHash;
		std::vector<u64> hashes1(inputs.size());
		std::vector<u64> hashes2(inputs.size());


		for (int idxItem = 0; idxItem < inputs.size(); idxItem++)
		{
			block cipher = mAesHasher.ecbEncBlock(inputs[idxItem]);
			u64 hh1 = _mm_extract_epi64(cipher, 0);
			u64 hh2 = _mm_extract_epi64(cipher, 1);

			hashes1[idxItem] = hh1 % mNumBins; //1st 64 bits for finding bin location
			hashes2[idxItem] = hh2 % mNumBins; //2nd 64 bits for finding alter bin location

			//if (hashes1[idxItem] > hashes2[idxItem])
			increasingSwap(hashes1[idxItem], hashes2[idxItem]);
			
			//std::cout << "hashes: " << hashes1[idxItem] << " === " << hashes2[idxItem] << std::endl;

			std::string key = concateInts(hashes1[idxItem] , hashes2[idxItem] , mNumBins); //h1||h2

			auto  p = mEdgeIdxMap.find(key);

			if (p == mEdgeIdxMap.end())
			{
				//groupHash.emplace(key, std::vector<int>{ idxItem });
				mEdgeIdxMap.emplace(pair<string, int>(key, idxItem));
				std::cout << "mEdgeIdxMap: " << key << " === " << idxItem << std::endl;
			}
			else
			{
				mIdx_inputs_circle_contains_2vertices.push_back(idxItem);
			}
		}

		std::cout << "mEdgeIdxMap.size(): " << mEdgeIdxMap.size() << std::endl;

		ccGraph g(mEdgeIdxMap.size()); //init vertext 0,,m

		//if 2 items have same h1||h2 or h2||h1, insert only 1, other push in stash
		for (auto it = mEdgeIdxMap.begin(); it != mEdgeIdxMap.end(); ++it)
		{
			u64 idxItem = it->second;
			boost::add_edge(hashes1[idxItem], hashes2[idxItem], g);
		}

		mCuckooGraph = g;
	}
}
