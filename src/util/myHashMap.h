/****************************************************************************
  FileName     [ myHash.h ]
  PackageName  [ util ]
  Synopsis     [ Define Hash and Cache ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2009-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_MAP_H
#define MY_HASH_MAP_H

#include <vector>

using namespace std;

// TODO: (Optionally) Implement your own Hash and Cache classes.

//-----------------------
// Define Hash classes
//-----------------------
// To use Hash ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.
//
// class HashKey
// {
// public:
//    HashKey() {}
// 
//    size_t operator() () const { return 0; }
// 
//    bool operator == (const HashKey& k) const { return true; }
// 
// private:
// };
//
class HashKey
{
public:
   HashKey(size_t a, size_t b)
   : _key( (a << 32) + (b << 8 ))
   { assert(a <= b); }

   size_t operator() () const { return _key; }

   bool operator == (const HashKey& k) const { return _key == k._key; }

   //bool invert(const HashKey& k) { return (_key ^ k._key) == 3; }

private:
   size_t _key;
};


template <class HashKey, class HashData>
class HashMap
{
typedef pair<HashKey, HashData> HashNode;

public:
   HashMap(size_t b=0) : _numBuckets(0), _buckets(0) { if (b != 0) init(b); }
   ~HashMap() { reset(); }

   // [Optional] TODO: implement the Hash<HashKey, HashData>::iterator
   // o An iterator should be able to go through all the valid HashNodes
   //   in the Hash
   // o Functions to be implemented:
   //   - constructor(s), destructor
   //   - operator '*': return the HashNode
   //   - ++/--iterator, iterator++/--
   //   - operators '=', '==', !="
   //
   class iterator
   {
      friend class HashMap<HashKey, HashData>;

   public:
      iterator(HashMap<HashKey, HashData>* h = 0, size_t b = 0, size_t bn = 0)
      : _hash(h), __bucketsNo(b), _nodesNo(bn) {}
      iterator(const iterator& i)
      : _hash(i._hash), __bucketsNo(i.__bucketsNo), _nodesNo(i._nodesNo) {}
      ~iterator() {} // Should NOT delete HashData

      const HashNode& operator * () const { return (*_hash)[__bucketsNo][_nodesNo]; }
      HashNode& operator * () { return (*_hash)[__bucketsNo][_nodesNo]; }
      iterator& operator ++ () {
         if (_hash == 0) return (*this);
         if (__bucketsNo >= _hash->_numBuckets) return (*this);
         if (++_nodesNo >= (*_hash)[__bucketsNo].size()) {
            while ((++__bucketsNo < _hash->_numBuckets) && (*_hash)[__bucketsNo].empty());
            _nodesNo = 0;
         }
         return (*this);
      }
      iterator& operator -- () {
         if (_hash == 0) return (*this);
         if (_nodesNo == 0) {
            if (__bucketsNo == 0) return (*this);
            while ((*_hash)[--__bucketsNo].empty())
               if (__bucketsNo == 0) return (*this);
            _nodesNo = (*_hash)[__bucketsNo].size() - 1;
         }
         else
            --_nodesNo;
         return (*this);
      }
      iterator operator ++ (int) { iterator li=(*this); ++(*this); return li; }
      iterator operator -- (int) { iterator li=(*this); --(*this); return li; }

      iterator& operator = (const iterator& i) {
         _hash = i._hash; __bucketsNo = i.__bucketsNo; _nodesNo = i._nodesNo; return (*this); }

      bool operator != (const iterator& i) const { return !(*this == i); }
      bool operator == (const iterator& i) const {
         return (_hash == i._hash && __bucketsNo == i.__bucketsNo && _nodesNo == i._nodesNo); }

   private:
      HashMap<HashKey, HashData>*   _hash;
      size_t                        __bucketsNo;
      size_t                        _nodesNo;
   };

   void init(size_t b) {
      reset(); _numBuckets = b; _buckets = new vector<HashNode>[b]; }
   void reset() {
      _numBuckets = 0;
      if (_buckets) { delete [] _buckets; _buckets = 0; }
   }
   void clear() {
      for (size_t i = 0; i < _numBuckets; ++i) _buckets[i].clear();
   }
   size_t numBuckets() const { return _numBuckets; }

   vector<HashNode>& operator [] (size_t i) { return _buckets[i]; }
   const vector<HashNode>& operator [](size_t i) const { return _buckets[i]; }

   // TODO: implement these functions
   //
   // Point to the first valid data
   iterator begin() const {
      if (_buckets == 0) return end();
      for (size_t i = 0; i < _numBuckets; ++i)
         if (!_buckets[i].empty())
            return iterator(const_cast<HashMap<HashKey, HashData>*>(this), i, 0);
      return end();
   }
   // Pass the end
   iterator end() const {
      return iterator(const_cast<HashMap<HashKey, HashData>*>(this),_numBuckets, 0);
   }
   // return true if no valid data
   bool empty() const {
      return begin()==end();
   }

   // number of valid data
   size_t size() const {
      size_t s = 0;
      for(HashMap::iterator i = begin(); i != end(); ++i) ++s;
      return s; 
   }

   // check if k is in the hash...
   // if yes, return true;
   // else return false;
   bool check(const HashKey& k, HashData& n) const {
      size_t b = bucketNum(k);
      for (size_t i = 0, bn = _buckets[b].size(); i < bn; ++i)
         if (_buckets[b][i].first == k) {
            n = _buckets[b][i].second;
            return true;
         }
      return false;
   }

   bool checkInv(const HashKey& k) const {
      size_t b = bucketNum(k);
      for (size_t i = 0, bn = _buckets[b].size(); i < bn; ++i)
         if (_buckets[b][i].first == k) {
            return k.invert(_buckets[b][i].first);
         }
      return false;
   }

   // return true if inserted successfully (i.e. k is not in the hash)
   // return false is k is already in the hash ==> will not insert
   bool insert(const HashKey& k, const HashData& d) {
      size_t b = bucketNum(k);
      for (size_t i = 0, bn = _buckets[b].size(); i < bn; ++i)
         if (_buckets[b][i].first == k)
            return false;
      _buckets[b].push_back(HashNode(k, d));
      return true;
   }

   // return true if inserted successfully (i.e. k is not in the hash)
   // return false is k is already in the hash ==> still do the insertion
   void forceInsert(const HashKey& k, const HashData& d) {
      _buckets[bucketNum(k)].push_back(HashNode(k, d)); 
   }
   // query if k is in the hash...
   // if yes, replace d with the data in the hash and return true;
   // else return false;
   bool query(const HashKey& k, HashData& d) const { return false; }

   // update the entry in hash that is equal to k (i.e. == return true)
   // if found, update that entry with d and return true;
   // else insert d into hash as a new entry and return false;
   bool update(const HashKey& k, HashData& d) { return false; }

   // return true if inserted d successfully (i.e. k is not in the hash)
   // return false is k is already in the hash ==> will not insert
   //bool insert(const HashKey& k, const HashData& d) { return true; }

   // return true if removed successfully (i.e. k is in the hash)
   // return fasle otherwise (i.e. nothing is removed)
   bool remove(const HashKey& k) { return false; }

private:
   // Do not add any extra data member
   size_t                   _numBuckets;
   vector<HashNode>*        _buckets;

   size_t bucketNum(const HashKey& k) const {
      return (k() % _numBuckets); }

};


//---------------------
// Define Cache classes
//---------------------
// To use Cache ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.
//
// class CacheKey
// {
// public:
//    CacheKey() {}
//    
//    size_t operator() () const { return 0; }
//   
//    bool operator == (const CacheKey&) const { return true; }
//       
// private:
// }; 
// 
template <class CacheKey, class CacheData>
class Cache
{
typedef pair<CacheKey, CacheData> CacheNode;

public:
   Cache() : _size(0), _cache(0) {}
   Cache(size_t s) : _size(0), _cache(0) { init(s); }
   ~Cache() { reset(); }

   // NO NEED to implement Cache::iterator class

   // TODO: implement these functions
   //
   // Initialize _cache with size s
   void init(size_t s) { reset(); _size = s; _cache = new CacheNode[s]; }
   void reset() {  _size = 0; if (_cache) { delete [] _cache; _cache = 0; } }

   size_t size() const { return _size; }

   CacheNode& operator [] (size_t i) { return _cache[i]; }
   const CacheNode& operator [](size_t i) const { return _cache[i]; }

   // return false if cache miss
   bool read(const CacheKey& k, CacheData& d) const {
      size_t i = k() % _size;
      if (k == _cache[i].first) {
         d = _cache[i].second;
         return true;
      }
      return false;
   }
   // If k is already in the Cache, overwrite the CacheData
   void write(const CacheKey& k, const CacheData& d) {
      size_t i = k() % _size;
      _cache[i].first = k;
      _cache[i].second = d;
   }

private:
   // Do not add any extra data member
   size_t         _size;
   CacheNode*     _cache;
};


#endif // MY_HASH_H
