#ifndef _MULTIMODAL_DATA_FORMAT__SLOTTED_PAGE_H_
#define _MULTIMODAL_DATA_FORMAT__SLOTTED_PAGE_H_
#include <cstring>
#include <cstdint>
#include <type_traits>
#include <memory>
#include "mpl.h"

/* ---------------------------------------------------------------
**
** Original - LibGStream - Library of GStream by InfoLab @ DGIST (https://infolab.dgist.ac.kr/)
** 
** Modified - LibMDF - Library of MDF by infoLab @ DGIST (https://infolab.dgist.ac.kr/)
** universal_slotted_page.h
** Generic C++ implementation of the "Slotted Page" which is a
** data structure for graph-data processing
**
** Author: Seyeon Oh (vee@dgist.ac.kr)
** Editor: Sanghyeon Lee (sh_lee@dgist.ac.kr)
** ------------------------------------------------------------ */

/* ---------------------------------------------------------------
**
** Universal Slotted Page represntation (Page size: user defined)
** +-------------------------------------------------------------+
** |                                                             |
** |                                                             |
** |                      Data Section                           |
** |               size = page size - footer size       +--------+
** |                                                    | footer |
** +----------------------------------------------------+--------+
**
** Detailed Universal slotted page representation
** +-------------------------------------------------------------+
** | S0 record size | S0 adj-elem #0 | S0 adj-elem #1 |   S0 adj-|
** +-------------------------------------------------------------+
** |elem #2 | S0 adj-elem #3 | ...                               |
** +-------------------------------------------------------------+
** |                                                             |
** +-------------------------------------------------------------+
** |                ... | S0 adj-elem #N | S1 record size |   S1 |
** +-------------------------------------------------------------+
** | adj-elem #0 | S2 adj-elem #1 | S1 adj-elem #2 | S1 adj-elem |
** +-------------------------------------------------------------+
** | #3 | ...                                                    |
** +-------------------------------------------------------------+
** |                                                             |
** +-------------------------------------------------------------+
** |                                    ...| S1 adj-elem #M |    |
** +-------------------------------------------------------------+
** |                                                             |
** +-------------------------------------------------------------+
** |                 | slot #1 (S1) | slot #0 (S0) | page footer |
** +-------------------------------------------------------------+
**
** Adjacency list-size: user defined type
** +--------------------------------------------+
** | Slot # adjacency list size (user-def type) |
** +--------------------------------------------+
**
** Adjacency list-element (Record) representation
** +-----------------------------------------------------------------------------------------+
** | data_info (user-def type) | data_size (user-def type) | data-payload (user-def type)	 |
** +-----------------------------------------------------------------------------------------+
**
** Slot representation
** +--------------------------------------------------------------------------------------------+
** | serial_id (user-def type) | record_offset (user-def type) | key-payload (user-def type)	|
** +--------------------------------------------------------------------------------------------+
**
** Page footer representation: default 16 bytes
** @offset_t: user-def type, default = uint32_t
** +--------------------------------------------------------------------------+
** | reserved (4byte) | flags (uint32_t) | front (offset_t) | rear (offset_t) |
** +--------------------------------------------------------------------------+
**
** ------------------------------------------------------------ */

namespace mdf {

constexpr size_t SIZE_1KB = 1024u;
constexpr size_t SIZE_1MB = SIZE_1KB * 1024u;
constexpr size_t SIZE_1GB = SIZE_1MB * 1024u;


/*
Record -
record_size - how many data_payload (fixed size)
data_info - what kind of extension type (fixed size)
data_size - value_payload list size (fixed size)
data_payload - it is for containing multi-modal data (each VP is variable size)

Slot -
serial_id - it is unique to identify slot
record_offset - it is used for finding record location
key_payload - it is not defined

etc. -
page_size - same
offset - footer ( meta-data )
*/
#define __MDF_SLOTTED_PAGE_TEMPLATE \
template <\
    typename __serial_id_t,\
    typename __data_info_t,\
    typename __record_offset_t,\
    typename __data_size_t,\
    typename __record_size_t,\
    size_t   __page_size,\
    typename __data_payload_t,\
    typename __key_payload_t,\
    typename __offset_t\
>

#define __MDF_SLOTTED_PAGE_TEMPLATE_ARGS \
    __serial_id_t,\
    __data_info_t,\
    __record_offset_t,\
    __data_size_t,\
    __record_size_t,\
    __page_size,\
    __data_payload_t,\
    __key_payload_t,\
    __offset_t

//flag 정의
namespace slotted_page_flag {
constexpr uint32_t _BASE = 0x0001;
constexpr uint32_t SP = _BASE;
constexpr uint32_t LP_HEAD = _BASE << 1;
constexpr uint32_t LP_EXTENDED = _BASE << 2;
} // !namespace slotted_page_flag

  //나중에 여러 타입에 따라 reserve에 박아 넣을 타입 플래그
namespace slotted_page_type {
	constexpr uint32_t _BASE = 0x0001;
	constexpr uint32_t IMAGE = _BASE;
	constexpr uint32_t LP_HEAD = _BASE << 1;
	constexpr uint32_t LP_EXTENDED = _BASE << 2;
} // !namespace slotted_page_type


namespace _slotted_page {

using default_offset_t = uint32_t;
using default_data_size_t = size_t;
using default_record_offset_t = size_t;
using default_serial_id_t = uint64_t;
using default_record_size_t = size_t;
using default_data_payload_t = char[1];
constexpr size_t default_page_size = SIZE_1MB * 64;
#pragma pack (push, 1)

template <typename __data_info_t,
    typename __data_size_t,
    typename __data_payload_t>
    struct adj_list_element
{
    __data_info_t      data_info;
    __data_size_t  data_size;
    __data_payload_t payload;
};

template <
    typename __data_size_t,
    typename __data_payload_t>
    struct adj_list_element<void, __data_size_t, __data_payload_t> {
    
    __data_size_t  data_size;
	__data_payload_t payload;
};

template <
    typename __serial_id_t,
    typename __record_offset_t,
    typename __key_payload_t>
    struct slot {
    __serial_id_t      serial_id;
    __record_offset_t  record_offset;
    __key_payload_t key_payload;
};

template <
    typename __serial_id_t,
    typename __record_offset_t>
    struct slot<__serial_id_t, __record_offset_t, void> {
    __serial_id_t     serial_id;
    __record_offset_t record_offset;
};

using page_flag_t = uint32_t;

template <typename offset_t>
struct footer {
    uint32_t    reserved;
    page_flag_t flags;
    offset_t    front;
    offset_t    rear;
};

#pragma pack (pop)

} // !namespace _slotted_page

#define __MDF_SLOTTED_PAGE_TEMPLATE_TYPEDEFS \
    using serial_id_t = __serial_id_t;\
    using data_info_t = __data_info_t;\
    using record_offset_t = __record_offset_t;\
    using data_size_t = __data_size_t;\
    using record_size_t = __record_size_t;\
    using data_payload_t = __data_payload_t;\
    using key_payload_t = __key_payload_t;\
    using offset_t = __offset_t;\
    using page_flag_t = _slotted_page::page_flag_t;\
    using adj_list_elem_t = _slotted_page::adj_list_element<data_info_t, data_size_t, data_payload_t>;\
    using slot_t = _slotted_page::slot<serial_id_t, record_offset_t, key_payload_t>;\
    using footer_t = _slotted_page::footer<offset_t>;\
    using ___size_t = target_arch_size_t

#define ALIAS_SLOTTED_PAGE_TEMPLATE_TYPEDEFS(PAGE_T) \
    using serial_id_t = typename PAGE_T::serial_id_t;\
    using data_info_t = typename PAGE_T::data_info_t;\
    using record_offset_t = typename PAGE_T::record_offset_t;\
    using data_size_t = typename PAGE_T::data_size_t;\
    using record_size_t = typename PAGE_T::record_size_t;\
    using data_payload_t = typename PAGE_T::data_payload_t;\
    using key_payload_t = typename PAGE_T::key_payload_t;\
    using offset_t = typename PAGE_T::offset_t;\
    using page_flag_t = typename PAGE_T::page_flag_t;\
    using adj_list_elem_t = typename PAGE_T::adj_list_elem_t;\
    using slot_t = typename PAGE_T::slot_t;\
    using footer_t = typename PAGE_T::footer_t;\
    using ___size_t = typename PAGE_T::___size_t


//EdgePayloadSize 사용불가. 가변길이 스트럭쳐라서.
//사이즈 받아오는 영역인듯 근데 못받는데??..
//    static constexpr ___size_t EdgePayloadSize = mpl::_sizeof<data_payload_t>::value;\
//static constexpr ___size_t MaximumEdgesInHeadPage = (DataSectionSize - sizeof(slot_t) - sizeof(record_size_t)) / sizeof(adj_list_elem_t);\
 //   static constexpr ___size_t MaximumEdgesInExtPage = (DataSectionSize - sizeof(slot_t)) / sizeof(adj_list_elem_t);\
//static constexpr ___size_t VertexPayloadSize = mpl::_sizeof<key_payload_t>::value;\

#define __MDF_SLOTTED_PAGE_TEMPLATE_CONSTDEFS \
    static constexpr ___size_t PageSize = __page_size;\
    static constexpr ___size_t DataSectionSize = PageSize - sizeof(footer_t);\
    static constexpr ___size_t SlotSize = sizeof(slot_t)

//    static constexpr ___size_t EdgePayloadSize = PAGE_T::EdgePayloadSize;\
//    static constexpr ___size_t MaximumEdgesInHeadPage = PAGE_T::MaximumEdgesInHeadPage;\
//    static constexpr ___size_t MaximumEdgesInExtPage = PAGE_T::MaximumEdgesInExtPage;\
 //   static constexpr ___size_t VertexPayloadSize = PAGE_T::VertexPayloadSize;\

#define ALIAS_SLOTTED_PAGE_TEMPLATE_CONSTDEFS(PAGE_T) \
    static constexpr ___size_t PageSize = PAGE_T::PageSize;\
    static constexpr ___size_t DataSectionSize = PAGE_T::DataSectionSize;\
    static constexpr ___size_t SlotSize = PAGE_T::SlotSize


//기본 세팅 - 슬롯 1 - 레코드 1 ( 그래서 레코드 사이즈 필요없다. 처리 두가지 필요)
#pragma pack (push, 1)
template <
    typename __serial_id_t = _slotted_page::default_serial_id_t,
    typename __data_info_t = void,
    typename __record_offset_t = _slotted_page::default_record_offset_t,
    typename __data_size_t = _slotted_page::default_data_size_t,
    typename __record_size_t = _slotted_page::default_record_size_t,
    size_t   __page_size = _slotted_page::default_page_size,
    typename __data_payload_t = _slotted_page::default_data_payload_t,
    typename __key_payload_t = void,
    typename __offset_t = _slotted_page::default_offset_t
>
class slotted_page {

    /* Constratint 1. The edge-payload type must be a void type or a Plain old data (POD) type */
    //static_assert((std::is_void<__data_payload_t>::value || std::is_pod<__data_payload_t>::value),
    //             "Generic Slotted Page: Constraint 1. The edge-payload type must be a Plain old data (POD) type");
    /* Constratint 2. The vertex-payload type must be a void type or a Plain old data (POD) type */
    //static_assert((std::is_void<__key_payload_t>::value || std::is_pod<__key_payload_t>::value),
    //             "Generic Slotted Page: Constraint 2. The vertex-payload type must be a Plain old data (POD) type");

    /* Typedefs and Constant value definitions */
public:
    using type = slotted_page<__MDF_SLOTTED_PAGE_TEMPLATE_ARGS>;
    using shared_ptr = std::shared_ptr<type>;
    using unique_ptr = std::unique_ptr<type>;
    using weak_ptr = std::weak_ptr<type>;
    __MDF_SLOTTED_PAGE_TEMPLATE_TYPEDEFS;
    __MDF_SLOTTED_PAGE_TEMPLATE_CONSTDEFS;

    /* Member functions */
        // Constructors & Destructor
    slotted_page() = default;
    explicit slotted_page(page_flag_t flag);
    slotted_page(const type& other);
    ~slotted_page() = default;

    // Operators
    inline type& operator=(const type& other)
	{
		memmove(this, &other, PageSize);
		return *this;
	}

    inline uint8_t& operator[](offset_t offset)
    {
	    return data_section[offset];
    }
    inline bool operator==(const type& other)
    {
        return memcmp(this, &other, sizeof(PageSize)) == 0;
    }

    // Utilites
    inline offset_t number_of_slots() const
    {
        return static_cast<offset_t>((DataSectionSize - this->footer.rear) / sizeof(slot_t));
    }
    inline slot_t& slot(const offset_t offset) 
    {
        return *reinterpret_cast<slot_t*>(&this->data_section[DataSectionSize - (sizeof(slot_t) * (offset + 1))]);
    }
	inline adj_list_elem_t* record_start(const slot_t& slot)
	{
		return reinterpret_cast<adj_list_elem_t*>(&data_section[slot.record_offset]);
	}
	//레코드 갯수
    inline record_size_t& record_size(const slot_t& slot) 
    {
        return *reinterpret_cast<record_size_t*>(&data_section[slot.record_offset]);
    }
    inline record_size_t& record_size(const offset_t slot_offset) 
    {
        return this->record_size(slot(slot_offset));
    }
	inline adj_list_elem_t* front()
	{//원래는 엣지 리스트 가져오는거임 아직 필요성 모르겠음.
		return reinterpret_cast<adj_list_elem_t*>(&data_section[footer.front]);
	}
	/*
    inline adj_list_elem_t* list(const slot_t& slot) 
    {//원래는 엣지 리스트 가져오는거임 아직 필요성 모르겠음.
        return reinterpret_cast<adj_list_elem_t*>(&data_section[slot.record_offset + sizeof(record_size_t)]);
    }
    inline adj_list_elem_t* list(const offset_t slot_offset) 
    {
        return reinterpret_cast<adj_list_elem_t*>(&data_section[slot(slot_offset).record_offset + sizeof(record_size_t)]);
    }
    inline adj_list_elem_t* list_ext(const slot_t& slot)
    {
        return reinterpret_cast<adj_list_elem_t*>(&data_section[slot.record_offset]);
    }
    inline adj_list_elem_t* list_ext(const offset_t slot_offset)
    {
        return reinterpret_cast<adj_list_elem_t*>(&data_section[slot(slot_offset).record_offset]);
    }
	*/
    inline page_flag_t& flags()
    {
        return footer.flags;
    }

    inline bool is_lp() const
    {
        return 0 != (footer.flags & (slotted_page_flag::LP_HEAD | slotted_page_flag::LP_EXTENDED));
    }
    inline bool is_lp_head() const
    {
        return 0 != (footer.flags & slotted_page_flag::LP_HEAD);
    }
    inline bool is_lp_extended() const
    {
        return 0 != (footer.flags & slotted_page_flag::LP_EXTENDED);
    }
    inline bool is_sp() const
    {
        return 0 != (footer.flags & slotted_page_flag::SP);
    }
    inline bool is_empty() const
    {
        return (footer.front == 0 && footer.rear == DataSectionSize);
    }

    /* Member variables */
public:
    uint8_t  data_section[DataSectionSize];
    footer_t footer{ 0, 0, 0, DataSectionSize };
};

#define __MDF_SLOTTED_PAGE slotted_page<__MDF_SLOTTED_PAGE_TEMPLATE_ARGS>

__MDF_SLOTTED_PAGE_TEMPLATE __MDF_SLOTTED_PAGE::slotted_page(const type& other)
{
    memmove(this, &other, PageSize);
}

__MDF_SLOTTED_PAGE_TEMPLATE __MDF_SLOTTED_PAGE::slotted_page(page_flag_t flags)
{
    footer.flags = flags;
}

#pragma pack (pop)

#pragma pack (push, 1)
template <
    typename __serial_id_t,
    typename __data_info_t,
    typename __record_offset_t,
    typename __data_size_t,
    typename __record_size_t,
    size_t   __page_size,
    typename __data_payload_t = void,
    typename __key_payload_t = void,
    typename __offset_t = _slotted_page::default_offset_t
> // 수정해야 될 부분 slotted page builder.
class slotted_page_builder: public slotted_page<__MDF_SLOTTED_PAGE_TEMPLATE_ARGS> {
public:
    using page_t = slotted_page<__MDF_SLOTTED_PAGE_TEMPLATE_ARGS>;
    using type = slotted_page_builder<__MDF_SLOTTED_PAGE_TEMPLATE_ARGS>;
    using shared_ptr = std::shared_ptr<type>;
    using unique_ptr = std::unique_ptr<type>;
    using weak_ptr = std::weak_ptr<type>;
    __MDF_SLOTTED_PAGE_TEMPLATE_TYPEDEFS;
    __MDF_SLOTTED_PAGE_TEMPLATE_CONSTDEFS;

    // Constructors & Destructor
    slotted_page_builder() = default;
    explicit slotted_page_builder(page_flag_t flag);
    slotted_page_builder(const type& other);
    ~slotted_page_builder() = default;

    // Operators
    inline type& operator=(const type& other);
    inline uint8_t& operator[](offset_t offset)
    {
        return this->data_section[offset];
    }
    inline bool operator==(const type& other)
    {
        return memcmp(this, &other, sizeof(PageSize)) == 0;
    }

    /// Scan: Scan the free space of a page and returns a couple of information as follows
    // (1) Whether this page can store a new slot. <Boolean>
    // (2) An available record size of the new slot. (If (1) is false, the value is 0.) <page_cnt_t:size-type>
    std::pair<bool/* (1) */, ___size_t /* (2) */> scan() const;
    /// Scan for extended page
    std::pair<bool/* (1) */, ___size_t /* (2) */> scan_ext() const;

#if 0 // for CUDA(nvcc) compatiblity
    /// Add slot: Add a new slot into a page, returns an offset of new slot
    // Enabled if key_payload_t is void type.
    template <typename PayloadTy = key_payload_t>
	typename std::enable_if<std::is_void<PayloadTy>::value, offset_t>::type add_slot(serial_id_t vertex_id);

    // Enabled if key_payload_t is non-void type.
    template <typename PayloadTy = key_payload_t>
    offset_t add_slot(serial_id_t vertex_id, typename std::enable_if<!std::is_void<PayloadTy>::value, key_payload_t>::type payload);

    /// Add slot for an extened page
    // Enabled if key_payload_t is void type.
    template <typename PayloadTy = key_payload_t>
    typename std::enable_if<std::is_void<PayloadTy>::value, offset_t>::type add_slot_ext(serial_id_t vertex_id);
    // Enabled if key_payload_t is non-void type.
    template <typename PayloadTy = key_payload_t>
    offset_t add_slot_ext(serial_id_t vertex_id, typename std::enable_if<!std::is_void<PayloadTy>::value, key_payload_t>::type payload);
#endif 


	// 임시로 닫아놓음

	/// Add slot: Add a new slot into a page, returns an offset of new slot
	// Enabled if key_payload_t is void type.
	template <typename PayloadTy = key_payload_t>
	typename std::enable_if<std::is_void<PayloadTy>::value, offset_t>::type add_slot(serial_id_t serial_id)
	{
		/*  */
		this->footer.rear -= sizeof(slot_t); // 슬롯 크기만큼 이동
		//할당
		slot_t& slot = reinterpret_cast<slot_t&>(this->data_section[this->footer.rear]);
		slot.record_offset = static_cast<record_offset_t>(this->footer.front);
		slot.serial_id = serial_id;
		//this->footer.front += sizeof(record_size_t);
		return this->number_of_slots() - 1;
	}
	// Enabled if key_payload_t is void type.
	/*template <typename PayloadTy = key_payload_t>
	typename std::enable_if<std::is_void<PayloadTy>::value, offset_t>::type add_record(data_size_t data_size, data_payload_t data)
	{
		//this->footer.rear -= sizeof(slot_t);
		record_t& slot = reinterpret_cast<record_t&>(this->data_section[this->footer.front]);
		slot.record_offset = static_cast<record_offset_t>(this->footer.front);
		slot.vertex_id = vertex_id;
		//this->footer.front += sizeof(record_size_t);
		return this->number_of_slots() - 1;
	}*/
	// Enabled if key_payload_t is non-void type.
	template <typename PayloadTy = key_payload_t>
	offset_t add_slot(serial_id_t vertex_id, typename std::enable_if<!std::is_void<PayloadTy>::value, key_payload_t>::type payload)
	{
		this->footer.rear -= sizeof(slot_t);
		slot_t& slot = reinterpret_cast<slot_t&>(this->data_section[this->footer.rear]);
		slot.vertex_id = vertex_id;
		slot.record_offset = static_cast<record_offset_t>(this->footer.front);
		slot.vertex_payload = payload;
		//this->footer.front += sizeof(record_size_t);
		return this->number_of_slots() - 1;
	}
	/*
	/// Add slot for an extened page
	// Enabled if key_payload_t is void type.
	template <typename PayloadTy = key_payload_t>
	typename std::enable_if<std::is_void<PayloadTy>::value, offset_t>::type add_slot_ext(serial_id_t vertex_id)
	{
		this->footer.rear -= sizeof(slot_t);
		slot_t& slot = reinterpret_cast<slot_t&>(this->data_section[this->footer.rear]);
		slot.vertex_id = vertex_id;
		slot.record_offset = static_cast<record_offset_t>(this->footer.front);
		return this->number_of_slots() - 1;
	}

	// Enabled if key_payload_t is non-void type.
	template <typename PayloadTy = key_payload_t>
	offset_t add_slot_ext(serial_id_t vertex_id, typename std::enable_if<!std::is_void<PayloadTy>::value, key_payload_t>::type payload)
	{
		this->footer.rear -= sizeof(slot_t);
		slot_t& slot = reinterpret_cast<slot_t&>(this->data_section[this->footer.rear]);
		slot.vertex_id = vertex_id;
		slot.record_offset = static_cast<record_offset_t>(this->footer.front);
		slot.vertex_payload = payload;
		return this->number_of_slots() - 1;
	} 
*/
// 입시로 닫아놓음

    /// Add dummy slot
  //  offset_t add_dummy_slot();
 //   offset_t add_dummy_slot_ext();

    /// Add list: Add a adjacency list to slot[offset]
    /// Add list for a Small Page (SP)
    void add_list_sp(adj_list_elem_t& elem_arr);

    /// Add list for a head of Large Pages (LP-head)
  //  void add_list_lp_head(___size_t record_size, adj_list_elem_t* elem_arr, ___size_t num_elems_in_page);
    /// Add list for extended part of Large pages (LP-ext)
 //   void add_list_lp_ext(adj_list_elem_t* elem_arr, ___size_t num_elems_in_page);

    /// Add dummy list
  //  void add_dummy_list_sp(offset_t slot_offset, ___size_t record_size);
  //  void add_dummy_list_lp_head(___size_t record_size, ___size_t num_elems_in_page);
  //  void add_dummy_list_lp_ext(___size_t num_elems_in_page);

    /// Utilites
    void clear();
};

#define __MDF_SLOTTED_PAGE_BUILDER slotted_page_builder<__MDF_SLOTTED_PAGE_TEMPLATE_ARGS>



__MDF_SLOTTED_PAGE_TEMPLATE
__MDF_SLOTTED_PAGE_BUILDER::slotted_page_builder(const type& other)
{
    memmove(this, &other, PageSize);
}

__MDF_SLOTTED_PAGE_TEMPLATE
__MDF_SLOTTED_PAGE_BUILDER::slotted_page_builder(page_flag_t flags)
{
    this->footer.flags = flags;
}

__MDF_SLOTTED_PAGE_TEMPLATE
inline typename __MDF_SLOTTED_PAGE_BUILDER::type& __MDF_SLOTTED_PAGE_BUILDER::operator=(const type& other)
{
    memmove(this, &other, PageSize);
    return *this;
}

__MDF_SLOTTED_PAGE_TEMPLATE
std::pair<bool/* (1) */, typename __MDF_SLOTTED_PAGE_BUILDER::___size_t /* (2) */> __MDF_SLOTTED_PAGE_BUILDER::scan() const
{
    auto free_space = this->footer.rear - this->footer.front;
    if (free_space < (sizeof(slot_t) + sizeof(adj_list_elem_t)))
        return std::make_pair(false, 0); // The page does not have enough space to store new slot.
    free_space -= (sizeof(slot_t) + sizeof(adj_list_elem_t));
    return std::make_pair(true, static_cast<___size_t>(free_space));
}

__MDF_SLOTTED_PAGE_TEMPLATE
std::pair<bool/* (1) */, typename __MDF_SLOTTED_PAGE_BUILDER::___size_t /* (2) */> __MDF_SLOTTED_PAGE_BUILDER::scan_ext() const
{
    auto free_space = this->footer.rear - this->footer.front;
    if (free_space < sizeof(slot_t)) //! Extended page does not need to space to store adjacency list size.
        return std::make_pair(false, 0); // The page does not have enough space to store new slot.
    free_space -= (sizeof(slot_t));
    return std::make_pair(true, static_cast<___size_t>(free_space / sizeof(adj_list_elem_t)));
}



#if 0 // for CUDA(nvcc) compatibility
__MDF_SLOTTED_PAGE_TEMPLATE
template <typename PayloadTy>
typename std::enable_if<std::is_void<PayloadTy>::value, typename __MDF_SLOTTED_PAGE_BUILDER::offset_t>::type __MDF_SLOTTED_PAGE_BUILDER::add_slot(serial_id_t vertex_id)
{
    this->footer.rear -= sizeof(slot_t);
    slot_t& slot = reinterpret_cast<slot_t&>(this->data_section[this->footer.rear]);
    slot.record_offset = static_cast<record_offset_t>(this->footer.front);
    slot.vertex_id = vertex_id;
    this->footer.front += sizeof(record_size_t);
    return this->number_of_slots() - 1;
}

__MDF_SLOTTED_PAGE_TEMPLATE
template <typename PayloadTy>
typename __MDF_SLOTTED_PAGE_BUILDER::offset_t __MDF_SLOTTED_PAGE_BUILDER::add_slot(serial_id_t vertex_id, typename std::enable_if<!std::is_void<PayloadTy>::value, key_payload_t>::type payload)
{
    this->footer.rear -= sizeof(slot_t);
    slot_t& slot = reinterpret_cast<slot_t&>(this->data_section[this->footer.rear]);
    slot.vertex_id = vertex_id;
    slot.record_offset = static_cast<record_offset_t>(this->footer.front);
    slot.key_payload_t = payload;
    this->footer.front += sizeof(record_size_t);
    return this->number_of_slots() - 1;
}

__MDF_SLOTTED_PAGE_TEMPLATE
template <typename PayloadTy>
typename std::enable_if<std::is_void<PayloadTy>::value, __MDF_SLOTTED_PAGE_BUILDER::offset_t>::type __MDF_SLOTTED_PAGE_BUILDER::add_slot_ext(serial_id_t vertex_id)
{
    this->footer.rear -= sizeof(slot_t);
    slot_t& slot = reinterpret_cast<slot_t&>(this->data_section[this->footer.rear]);
    slot.vertex_id = vertex_id;
    slot.record_offset = static_cast<record_offset_t>(this->footer.front);
    return this->number_of_slots() - 1;
}

__MDF_SLOTTED_PAGE_TEMPLATE
template <typename PayloadTy>
typename __MDF_SLOTTED_PAGE_BUILDER::offset_t __MDF_SLOTTED_PAGE_BUILDER::add_slot_ext(serial_id_t vertex_id, typename std::enable_if<!std::is_void<PayloadTy>::value, key_payload_t>::type payload)
{
    this->footer.rear -= sizeof(slot_t);
    slot_t& slot = reinterpret_cast<slot_t&>(this->data_section[this->footer.rear]);
    slot.vertex_id = vertex_id;
    slot.record_offset = static_cast<record_offset_t>(this->footer.front);
    slot.key_payload_t = payload;
    return this->number_of_slots() - 1;
}
#endif
/*
__MDF_SLOTTED_PAGE_TEMPLATE
typename __MDF_SLOTTED_PAGE_BUILDER::offset_t __MDF_SLOTTED_PAGE_BUILDER::add_dummy_slot()
{
    this->footer.rear -= sizeof(slot_t);
    this->footer.front += sizeof(record_size_t);
    return this->number_of_slots() - 1;
}

__MDF_SLOTTED_PAGE_TEMPLATE
typename __MDF_SLOTTED_PAGE_BUILDER::offset_t __MDF_SLOTTED_PAGE_BUILDER::add_dummy_slot_ext()
{
    this->footer.rear -= sizeof(slot_t);
    return this->number_of_slots() - 1;
}*/

__MDF_SLOTTED_PAGE_TEMPLATE
void __MDF_SLOTTED_PAGE_BUILDER::add_list_sp(adj_list_elem_t& elem_arr)
{
	// 여기 수정해야됩니다!!
    //slot_t& slot = this->slot(slot_offset);
    //this->record_size(slot) = static_cast<record_size_t>(record_size);
    memmove(this->front(), &elem_arr, sizeof(adj_list_elem_t) + elem_arr.data_size);
    this->footer.front += static_cast<decltype(this->footer.front)>(sizeof(adj_list_elem_t)+ elem_arr.data_size);
	
}
/*
__MDF_SLOTTED_PAGE_TEMPLATE
void __MDF_SLOTTED_PAGE_BUILDER::add_list_lp_head(___size_t record_size, adj_list_elem_t* elem_arr, ___size_t num_elems_in_page)
{
    slot_t& slot = this->slot(0);
    this->record_size(slot) = static_cast<record_size_t>(record_size);
    memmove(this->list(slot), elem_arr, sizeof(adj_list_elem_t) * num_elems_in_page);
    this->footer.front += static_cast<decltype(this->footer.front)>(sizeof(adj_list_elem_t) * num_elems_in_page);
}*/
/*
__MDF_SLOTTED_PAGE_TEMPLATE
void __MDF_SLOTTED_PAGE_BUILDER::add_list_lp_ext(adj_list_elem_t* elem_arr, ___size_t num_elems_in_page)
{
slot_t& slot = this->slot(0);
memmove(this->list_ext(slot), elem_arr, sizeof(adj_list_elem_t) * num_elems_in_page);
this->footer.front += static_cast<decltype(this->footer.front)>(sizeof(adj_list_elem_t) * num_elems_in_page);
}
*/

/*
__MDF_SLOTTED_PAGE_TEMPLATE
void __MDF_SLOTTED_PAGE_BUILDER::add_dummy_list_sp(const offset_t slot_offset, ___size_t record_size)
{
    slot_t& slot = this->slot(slot_offset);
    this->record_size(slot.record_offset) = static_cast<record_size_t>(record_size);
    this->footer.front += static_cast<offset_t>(sizeof(adj_list_elem_t) * record_size);
}

__MDF_SLOTTED_PAGE_TEMPLATE
void __MDF_SLOTTED_PAGE_BUILDER::add_dummy_list_lp_head(___size_t record_size, ___size_t num_elems_in_page)
{
    slot_t& slot = this->slot(0);
    this->record_size(slot.record_offset) = record_size;
    this->footer.front += sizeof(adj_list_elem_t) * num_elems_in_page;
}

__MDF_SLOTTED_PAGE_TEMPLATE
void __MDF_SLOTTED_PAGE_BUILDER::add_dummy_list_lp_ext(___size_t num_elems_in_page)
{
    this->footer.front += sizeof(adj_list_elem_t) * num_elems_in_page;
}
*/
__MDF_SLOTTED_PAGE_TEMPLATE
void __MDF_SLOTTED_PAGE_BUILDER::clear()
{
    memset(this->data_section, 0, DataSectionSize);
    this->footer.front = 0;
    this->footer.rear = DataSectionSize;
}

//#undef __MDF_SLOTTED_PAGE_TEMPLATE_TYPEDEFS
//#undef __MDF_SLOTTED_PAGE_TEMPLATE_CONSTDEFS
#undef __MDF_SLOTTED_PAGE_BUILDER
#undef __MDF_SLOTTED_PAGE
//#undef __MDF_SLOTTED_PAGE_TEMPLATE_ARGS
//#undef __MDF_SLOTTED_PAGE_TEMPLATE

/* 이거 필요없음 
template <typename __builder_t, typename __rid_table_t>
typename __builder_t::data_info_t vid_to_pid(typename __builder_t::serial_id_t vid, __rid_table_t& table)
{
    for (typename __builder_t::___size_t i = 0; i < table.size(); ++i) {
        const auto& tuple = table[i];
        if (tuple.start_vid == vid)
            return static_cast<typename __builder_t::data_info_t>(i);
        if (tuple.start_vid > vid)
            return static_cast<typename __builder_t::data_info_t>(i - 1);
    }
    return static_cast<typename __builder_t::data_info_t>(table.size()) - 1;
}*/

//TODO: KNOWN ISSUE: UNSAFE CONVERSION
/*
template <typename __builder_t, typename __rid_table_t>
typename __builder_t::data_size_t get_slot_offset(typename __builder_t::data_info_t pid, typename __builder_t::serial_id_t vid, __rid_table_t& table)
{
    const auto& tuple = table[pid];
    return static_cast<typename __builder_t::data_size_t>(vid - tuple.start_vid);
}*/

/*
template <typename __serial_id_t, typename __payload_t = void>
struct edge_template
{
using serial_id_t = __serial_id_t;
using payload_t = __payload_t;
serial_id_t src;
serial_id_t dst;
payload_t   payload;
template <typename __builder_t, typename __rid_table_t>
void to_adj_elem(const __rid_table_t& table, typename __builder_t::adj_list_elem_t* out)
{
out->page_id = vid_to_pid<__builder_t>(dst, table);
out->slot_offset = get_slot_offset<__builder_t>(out->page_id, dst, table);
out->payload = payload;
}
};

template <typename __serial_id_t>
struct edge_template<__serial_id_t, void>
{
using serial_id_t = __serial_id_t;
using payload_t = void;
serial_id_t src;
serial_id_t dst;
template <typename __builder_t, typename __rid_table_t>
void to_adj_elem(const __rid_table_t& table, typename __builder_t::adj_list_elem_t* out)
{
out->page_id = vid_to_pid<__builder_t>(dst, table);
out->slot_offset =  get_slot_offset<__builder_t>(out->page_id, dst, table);
}
};
*/


template <typename __data_info_t, typename __data_size_t, typename __data_payload_t>
struct record_template
{
	using data_info = __data_info_t;
	using data_size_t = __data_size_t;
	using data_payload_t = __data_payload_t;

	data_info info;
	data_size_t size;
	data_payload_t data;

	//데이터 길이만큼 넣고 빼고 해야됨.
	template <typename __builder_t>
	void to_adj_elem(typename __builder_t::adj_list_elem_t* out)
	{
		out->data_info = info;
		out->data_size = size;
		memmove(out->payload, data, size);
		//out->page_id = vid_to_pid<__builder_t>(dst, table);
		//out->slot_offset = get_slot_offset<__builder_t>(out->page_id, dst, table);
	}

};
template <typename __data_size_t, typename __data_payload_t>
struct record_template<void, typename __data_size_t, typename __data_payload_t>
{
	using data_info = void;
	using data_size_t = __data_size_t;
	using data_payload_t = __data_payload_t;

	
	data_size_t size;
	data_payload_t data;

	//데이터 길이만큼 넣고 빼고 해야됨.
	//레코드로 바꿔주는 함수 
	template <typename __builder_t>
	void to_adj_elem(typename __builder_t::adj_list_elem_t* out)
	{
		out->data_size = size;
		memmove(out->payload, data, size);
			//out->page_id = vid_to_pid<__builder_t>(dst, table);
			//out->slot_offset = get_slot_offset<__builder_t>(out->page_id, dst, table);
	}

};

template <typename __serial_id_t, typename __payload_t = void>
struct vertex_template
{
    using serial_id_t = __serial_id_t;
    using payload_t = __payload_t;
    serial_id_t serial_id;
    payload_t   payload;
	//슬롯으로 바꿔주는 함수
	template <typename __builder_t>
	void to_slot(__builder_t& target_page) const
	{
		target_page.add_slot(vertex_id, payload);
	}
	template <typename __builder_t>
	void to_slot_ext(__builder_t& target_page) const
	{
		target_page.add_slot_ext(vertex_id, payload);
	}
};

template <typename __serial_id_t>
struct vertex_template<__serial_id_t, void>
{
    using serial_id_t = __serial_id_t;
    using payload_t = void;
    serial_id_t serial_id;
	//슬롯으로 바꿔주는 함수
	template <typename __builder_t>
	void to_slot(__builder_t& target_page) const
	{
		target_page.add_slot(serial_id);
	}
	template <typename __builder_t>
	void to_slot_ext(__builder_t& target_page) const
	{
		target_page.add_slot_ext(serial_id);
	}
};

#pragma pack(pop)

} // !namespace gstream

#endif // !_MULTIMODAL_DATA_FORMAT__SLOTTED_PAGE_H_