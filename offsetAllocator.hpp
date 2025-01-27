// (C) Sebastian Aaltonen 2023
// MIT License (see file: LICENSE)

//#define USE_16_BIT_NODE_INDICES

namespace OffsetAllocator
{
    typedef unsigned char uint8;
    typedef unsigned short uint16;
    typedef unsigned int uint32;

    // 16 bit offsets mode will halve the metadata storage cost
    // But it only supports up to 65536 maximum allocation count
#ifdef USE_16_BIT_NODE_INDICES
    typedef uint16 NodeIndex;
#else
    typedef uint32 NodeIndex;
#endif

    static constexpr uint32 NUM_TOP_BINS = 32;
    static constexpr uint32 BINS_PER_LEAF = 8;
    static constexpr uint32 TOP_BINS_INDEX_SHIFT = 3;
    static constexpr uint32 LEAF_BINS_INDEX_MASK = 0x7;
    static constexpr uint32 NUM_LEAF_BINS = NUM_TOP_BINS * BINS_PER_LEAF;

    struct Allocation
    {
        #ifdef USE_16_BIT_NODE_INDICES
        static constexpr NodeIndex NO_SPACE = 0xFFFF;
        #else
        static constexpr NodeIndex NO_SPACE = 0xFFFFFFFF;
        #endif
 
        uint32 offset = NO_SPACE;
        NodeIndex metadata = NO_SPACE; // internal: node index
    };

    struct StorageReport
    {
        uint32 totalFreeSpace;
        uint32 largestFreeRegion;
    };

    struct StorageReportFull
    {
        struct Region
        {
            uint32 size;
            uint32 count;
        };
        
        Region freeRegions[NUM_LEAF_BINS];
    };

    class Allocator
    {
    public:
        // Return the maximum allocation size supported when the allocator handle a range of 'size' elements
        static uint32 ComputeMaxAllocSize(uint32 size);

        // Return the minimum size of the range handled by the allocator that supports the given maximum allocation size.
        // 
        // Example:
        //    const uint32_t inputDataSize = ...;
        //    const uint32_t maxAllocSize = ...; // <= inputDataSize
        //    const uint32_t allocatorMinSize = OffsetAllocator::Allocator::ComputeSizeToAllowMaxAllocSize(maxAllocSize);
        //    const uint32_t actualAllocSize = inputDataSize > allocatorMinSize ? inputDataSize : allocatorMinSize;
        //
        //    const uint32_t allocatorMaxAllocSize = OffsetAllocator::Allocator::ComputeMaxAllocSize(actualAllocSize);
        //    assert(allocatorMaxAllocSize >= maxAllocSize);
        //
        //    T * data = allocate<T>(actualAllocSize);
        //
        //    OffsetAllocator::Allocator allocator;
        //    allocator.init(actualAllocSize, maxAllocCount);
        //    OffsetAllocator::Allocation alloc = allocator.allocate(maxAllocSize);
        //    assert(alloc.offset == 0);
        static uint32 ComputeSizeToAllowMaxAllocSize(uint32 maxAllocSize);

    public:
        #ifdef USE_16_BIT_NODE_INDICES
        static constexpr uint16 MAX_NUM_ALLOCS = 65535;
        static constexpr uint16 DEFAULT_MAX_NUM_ALLOCS = 65535;
        #else
        static constexpr uint32 MAX_NUM_ALLOCS = 0xFFFFFFFEu;
        static constexpr uint32 DEFAULT_MAX_NUM_ALLOCS = 128 * 1024;
        #endif

        Allocator();
        Allocator(uint32 size, NodeIndex maxAllocs = DEFAULT_MAX_NUM_ALLOCS);
        Allocator(Allocator &&other);
        ~Allocator();

        void init(uint32 size, NodeIndex maxAllocs = DEFAULT_MAX_NUM_ALLOCS);
        void reset();
        
        Allocation allocate(uint32 size);
        void free(Allocation allocation);

        uint32 getMaxAllocationSize() const;
        uint32 allocationSize(Allocation allocation) const;
        StorageReport storageReport() const;
        StorageReportFull storageReportFull() const;
        
    private:
        NodeIndex insertNodeIntoBin(uint32 size, uint32 dataOffset);
        void removeNodeFromBin(NodeIndex nodeIndex);

        struct Node
        {
            #ifdef USE_16_BIT_NODE_INDICES
            static constexpr NodeIndex unused = 0xFFFF;
            #else
            static constexpr NodeIndex unused = 0xFFFFFFFF;
            #endif
            
            uint32 dataOffset = 0;
            uint32 dataSize = 0;
            NodeIndex binListPrev = unused;
            NodeIndex binListNext = unused;
            NodeIndex neighborPrev = unused;
            NodeIndex neighborNext = unused;
            bool used = false; // TODO: Merge as bit flag
        };
    
        uint32 m_size;
        NodeIndex m_maxAllocs;
        uint32 m_freeStorage;

        uint32 m_usedBinsTop;
        uint8 m_usedBins[NUM_TOP_BINS];
        NodeIndex m_binIndices[NUM_LEAF_BINS];
                
        Node* m_nodes;
        NodeIndex* m_freeNodes;
        NodeIndex m_freeOffset;
    };
}
