///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2012-2017 DreamWorks Animation LLC
//
// All rights reserved. This software is distributed under the
// Mozilla Public License 2.0 ( http://www.mozilla.org/MPL/2.0/ )
//
// Redistributions of source code must retain the above copyright
// and license notice and the following restrictions and disclaimer.
//
// *     Neither the name of DreamWorks Animation nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// IN NO EVENT SHALL THE COPYRIGHT HOLDERS' AND CONTRIBUTORS' AGGREGATE
// LIABILITY FOR ALL CLAIMS REGARDLESS OF THEIR BASIS EXCEED US$250.00.
//
///////////////////////////////////////////////////////////////////////////

/// @file points/AttributeTopology.h
///
/// @authors Dan Bailey
///
/// @brief  Topology

#ifndef OPENVDB_POINTS_ATTRIBUTE_TOPOLOGY_HAS_BEEN_INCLUDED
#define OPENVDB_POINTS_ATTRIBUTE_TOPOLOGY_HAS_BEEN_INCLUDED

#include <openvdb/version.h>
#include <openvdb/MetaMap.h>
#include <openvdb/points/AttributeArray.h>

#include <limits>
#include <memory>
#include <vector>


namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {
namespace points {


// forward declaration
class AttributeSet;


/// @brief A primitive stores an array of polygons with the same vertex count,
/// where each polygon stores connectivity information for a referenced point set.
class OPENVDB_API Primitive
{
public:
    using Ptr = std::shared_ptr<Primitive>;
    using ConstPtr = std::shared_ptr<const Primitive>;
    using IndexArray = TypedAttributeArray<int>;

    Primitive(Index size, Index vertices);

    Index size() const;
    Index vertices() const;

    AttributeSet& attributeSet() { return *mAttributeSet; }
    const AttributeSet& attributeSet() const { return *mAttributeSet; }

    inline AttributeArray& attributeArray(const size_t pos);
    inline const AttributeArray& attributeArray(const size_t pos) const;
    const AttributeArray& constAttributeArray(const size_t pos) const;

    AttributeArray& attributeArray(const Name& attributeName);
    const AttributeArray& attributeArray(const Name& attributeName) const;
    const AttributeArray& constAttributeArray(const Name& attributeName) const;

    bool operator==(const Primitive& other) const;
    bool operator!=(const Primitive& other) const { return !this->operator==(other); }

    void write(std::ostream& os) const;
    void read(std::istream& is);

private:
    std::unique_ptr<AttributeSet>   mAttributeSet;
}; // class Primitive


/// @brief AttributeTopology holds an array of primitives.
class OPENVDB_API AttributeTopology
{
public:
    AttributeTopology();

    AttributeTopology(const AttributeTopology& rhs);

    bool empty() const { return mPrimitives.empty(); }

    Index size() const { return mPrimitives.size(); }

    void addPrimitive(Primitive::Ptr& primitive) {
        mPrimitives.push_back(primitive);
    }

    Primitive::Ptr primitive(const Index i) { return i < mPrimitives.size() ? mPrimitives[i] : Primitive::Ptr(); }
    Primitive::ConstPtr primitive(const Index i) const { return i < mPrimitives.size() ? mPrimitives[i] : Primitive::ConstPtr(); }

    bool operator==(const AttributeTopology& other) const;
    bool operator!=(const AttributeTopology& other) const { return !this->operator==(other); }

    void write(std::ostream& os) const;
    void read(std::istream& is);

private:
    std::vector<Primitive::Ptr> mPrimitives;
}; // class AttributeTopology


} // namespace points
} // namespace OPENVDB_VERSION_NAME
} // namespace openvdb

#endif // OPENVDB_POINTS_ATTRIBUTE_TOPOLOGY_HAS_BEEN_INCLUDED

// Copyright (c) 2012-2017 DreamWorks Animation LLC
// All rights reserved. This software is distributed under the
// Mozilla Public License 2.0 ( http://www.mozilla.org/MPL/2.0/ )
