// Copyright (c) 2015 G. Andrew Stone
// Copyright (c) 2016 The Bitcoin Unlimited developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#pragma once
#ifndef BITCOIN_UNLIMITED_H
#define BITCOIN_UNLIMITED_H

#include "leakybucket.h"
#include "net.h"
#include "stat.h"
#include "thinblock.h"
#include "consensus/validation.h"
#include "consensus/params.h"
#include "requestManager.h"
#include <univalue.h>
#include <vector>

enum {
    DEFAULT_MAX_GENERATED_BLOCK_SIZE = 1000000,
    DEFAULT_EXCESSIVE_ACCEPT_DEPTH = 4,
    DEFAULT_EXCESSIVE_BLOCK_SIZE = 16000000,
    DEFAULT_MAX_MESSAGE_SIZE_MULTIPLIER = 10,
    MAX_COINBASE_SCRIPTSIG_SIZE = 100,
    EXCESSIVE_BLOCK_CHAIN_RESET = 6*24,  // After 1 day of non-excessive blocks, reset the checker
};

class CBlock;
class CBlockIndex;
class CValidationState;
class CDiskBlockPos;
class CNode;
class CChainParams;

extern uint32_t blockVersion;  // Overrides the mined block version if non-zero
extern uint64_t maxGeneratedBlock;
extern unsigned int excessiveBlockSize;
extern unsigned int excessiveAcceptDepth;
extern unsigned int maxMessageSizeMultiplier;

extern std::vector<std::string> BUComments;  // BU005: Strings specific to the config of this client that should be communicated to other clients
extern std::string minerComment;  // An arbitrary field that miners can change to annotate their blocks

// BU - Xtreme Thinblocks Auto Mempool Limiter - begin section
/** The default value for -minrelaytxfee */
static const char DEFAULT_MINLIMITERTXFEE[] = "0.0";
/** The default value for -maxrelaytxfee */
static const char DEFAULT_MAXLIMITERTXFEE[] = "3.0";
/** The number of block heights to gradually choke spam transactions over */
static const unsigned int MAX_BLOCK_SIZE_MULTIPLIER = 3;
/** The minimum value possible for -limitfreerelay when rate limiting */
static const unsigned int DEFAULT_MIN_LIMITFREERELAY = 1;
// BU - Xtreme Thinblocks Auto Mempool Limiter - end section

// Replace Core's ComputeBlockVersion
int32_t UnlimitedComputeBlockVersion(const CBlockIndex* pindexPrev, const Consensus::Params& params,uint32_t nTime);

// process incoming unsolicited block
void HandleExpeditedBlock(CDataStream& vRecv,CNode* pfrom);

//! The largest block size that we have seen since startup
extern uint64_t nLargestBlockSeen; // BU - Xtreme Thinblocks

// Convert the BUComments to the string client's "subversion" string
extern void settingsToUserAgentString();
// Convert a list of client comments (typically BUcomments) and a custom comment into a string appropriate for the coinbase txn
// The coinbase size restriction is NOT enforced
extern std::string FormatCoinbaseMessage(const std::vector<std::string>& comments,const std::string& customComment);  

extern void UnlimitedSetup(void);
extern std::string UnlimitedCmdLineHelp();

// Called whenever a new block is accepted
extern void UnlimitedAcceptBlock(const CBlock& block, CValidationState& state, CBlockIndex* ppindex, CDiskBlockPos* dbp);

extern void UnlimitedLogBlock(const CBlock& block, const std::string& hash, uint64_t receiptTime);

// used during mining
extern bool TestConservativeBlockValidity(CValidationState& state, const CChainParams& chainparams, const CBlock& block, CBlockIndex* pindexPrev, bool fCheckPOW, bool fCheckMerkleRoot);

// Check whether this block is bigger in some metric than we really want to accept
extern bool CheckExcessive(const CBlock& block, uint64_t blockSize, uint64_t nSigOps, uint64_t nTx);

// Check whether this chain qualifies as excessive.
extern int isChainExcessive(const CBlockIndex* blk, unsigned int checkDepth = excessiveAcceptDepth);

// Check whether any block N back in this chain is an excessive block
extern int chainContainsExcessive(const CBlockIndex* blk, unsigned int goBack=0);

// RPC calls
extern UniValue settrafficshaping(const UniValue& params, bool fHelp);
extern UniValue gettrafficshaping(const UniValue& params, bool fHelp);
extern UniValue pushtx(const UniValue& params, bool fHelp);

extern UniValue getminingmaxblock(const UniValue& params, bool fHelp);
extern UniValue setminingmaxblock(const UniValue& params, bool fHelp);

extern UniValue getexcessiveblock(const UniValue& params, bool fHelp);
extern UniValue setexcessiveblock(const UniValue& params, bool fHelp);

// Get and set the custom string that miners can place into the coinbase transaction
extern UniValue getminercomment(const UniValue& params, bool fHelp);
extern UniValue setminercomment(const UniValue& params, bool fHelp);

// Get and set the generated (mined) block version.  USE CAREFULLY!
extern UniValue getblockversion(const UniValue& params, bool fHelp);
extern UniValue setblockversion(const UniValue& params, bool fHelp);

// RPC Return a list of all available statistics
extern UniValue getstatlist(const UniValue& params, bool fHelp);
// RPC Get a particular statistic
extern UniValue getstat(const UniValue& params, bool fHelp);

// RPC Set a node to receive expedited blocks from
UniValue expedited(const UniValue& params, bool fHelp);

// These variables for traffic shaping need to be globally scoped so the GUI and CLI can adjust the parameters
extern CLeakyBucket receiveShaper;
extern CLeakyBucket sendShaper;

// BUIP010 Xtreme Thinblocks:
extern bool HaveConnectThinblockNodes();
extern bool HaveThinblockNodes();
extern bool CheckThinblockTimer(uint256 hash);
extern void ClearThinblockTimer(uint256 hash);
extern bool IsThinBlocksEnabled();
extern bool CanThinBlockBeDownloaded(CNode* pto);
extern bool IsChainNearlySyncd();
extern void IsChainNearlySyncdInit();
extern bool fIsChainNearlySyncd;
extern void BuildSeededBloomFilter(CBloomFilter& memPoolFilter, std::vector<uint256>& vOrphanHashes, uint256 hash);
extern void LoadFilter(CNode *pfrom, CBloomFilter *filter);
extern void HandleBlockMessage(CNode *pfrom, const std::string &strCommand, CBlock &block, const CInv &inv);
extern void ConnectToThinBlockNodes();
extern void CheckNodeSupportForThinBlocks();
extern void SendXThinBlock(CBlock &block, CNode* pfrom, const CInv &inv);

extern bool CheckAndRequestExpeditedBlocks(CNode* pfrom);  // Checks to see if the node is configured in bitcoin.conf to be an expedited block source and if so, request them.
extern void SendExpeditedBlock(CXThinBlock& thinBlock,unsigned char hops, const CNode* skip=NULL);
extern void SendExpeditedBlock(const CBlock& block,const CNode* skip=NULL);
extern void HandleExpeditedRequest(CDataStream& vRecv,CNode* pfrom);
extern bool IsRecentlyExpeditedAndStore(const uint256& hash);

extern std::map<uint256, uint64_t> mapThinBlockTimer;

// statistics
void UpdateSendStats(CNode* pfrom, const char* strCommand, int msgSize, int64_t nTime);

void UpdateRecvStats(CNode* pfrom, const std::string& strCommand, int msgSize, int64_t nTimeReceived);
// txn mempool statistics
extern CStatHistory<unsigned int, MinValMax<unsigned int> > txAdded;
extern CStatHistory<uint64_t, MinValMax<uint64_t> > poolSize;


// Protocol changes:


enum {
  EXPEDITED_STOP   = 1,
  EXPEDITED_BLOCKS = 2,
  EXPEDITED_TXNS   = 4,
};

enum {
  EXPEDITED_MSG_HEADER   = 1,
  EXPEDITED_MSG_XTHIN    = 2,
};


#endif