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

/// @file points/AttributeTopology.cc

#include "AttributeTopology.h"
#include "AttributeSet.h"

namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {
namespace points {


Primitive::Primitive(Index size, Index vertices) :
    mAttributeSet()
{
    openvdb::NamePair positionType = TypedAttributeArray<openvdb::Vec3f>::attributeType();
    AttributeSet::Descriptor::Ptr descr = AttributeSet::Descriptor::create(positionType);

    mAttributeSet.reset(new AttributeSet(descr, size));

    mAttributeSet->appendAttribute("index", IndexArray::attributeType(), vertices);
}


Index
Primitive::size() const
{
    return this->constAttributeArray("index").size();
}


Index
Primitive::vertices() const
{
    return this->constAttributeArray("index").stride();
}


AttributeArray&
Primitive::attributeArray(const size_t pos)
{
    if (pos >= mAttributeSet->size())             OPENVDB_THROW(LookupError, "Attribute Out Of Range - " << pos);
    return *mAttributeSet->get(pos);
}

const AttributeArray&
Primitive::attributeArray(const size_t pos) const
{
    if (pos >= mAttributeSet->size())             OPENVDB_THROW(LookupError, "Attribute Out Of Range - " << pos);
    return *mAttributeSet->getConst(pos);
}

const AttributeArray&
Primitive::constAttributeArray(const size_t pos) const
{
    return this->attributeArray(pos);
}

AttributeArray&
Primitive::attributeArray(const Name& attributeName)
{
    const size_t pos = mAttributeSet->find(attributeName);
    if (pos == AttributeSet::INVALID_POS)         OPENVDB_THROW(LookupError, "Attribute Not Found - " << attributeName);
    return *mAttributeSet->get(pos);
}

const AttributeArray&
Primitive::attributeArray(const Name& attributeName) const
{
    const size_t pos = mAttributeSet->find(attributeName);
    if (pos == AttributeSet::INVALID_POS)         OPENVDB_THROW(LookupError, "Attribute Not Found - " << attributeName);
    return *mAttributeSet->getConst(pos);
}

const AttributeArray&
Primitive::constAttributeArray(const Name& attributeName) const
{
    return this->attributeArray(attributeName);
}


bool
Primitive::operator==(const Primitive& other) const
{
    if (this->mAttributeSet && other.mAttributeSet) {
        return *this->mAttributeSet == *other.mAttributeSet;
    }
    return false;
}


void
Primitive::write(std::ostream& os) const
{
    mAttributeSet->writeDescriptor(os, /*transient=*/false);
    mAttributeSet->writeMetadata(os, /*transient=*/false, /*paged=*/true);

    compression::PagedOutputStream pagedStream;
    pagedStream.setOutputStream(os);

    pagedStream.setSizeOnly(true);
    for (int i = 0; i < mAttributeSet->size(); i++) {
        const AttributeArray* array = mAttributeSet->getConst(i);
        array->writePagedBuffers(pagedStream, /*outputTransient*/false);
        pagedStream.flush();
    }

    pagedStream.setSizeOnly(false);
    for (int i = 0; i < mAttributeSet->size(); i++) {
        const AttributeArray* array = mAttributeSet->getConst(i);
        array->writePagedBuffers(pagedStream, /*outputTransient*/false);
        pagedStream.flush();
    }
}


void
Primitive::read(std::istream& is)
{
    mAttributeSet->readDescriptor(is);
    mAttributeSet->readMetadata(is);

    compression::PagedInputStream pagedStream;
    pagedStream.setInputStream(is);

    pagedStream.setSizeOnly(true);
    for (int i = 0; i < mAttributeSet->size(); i++) {
        AttributeArray* array = mAttributeSet->get(i);
        array->readPagedBuffers(pagedStream);
    }

    pagedStream.setSizeOnly(false);
    for (int i = 0; i < mAttributeSet->size(); i++) {
        AttributeArray* array = mAttributeSet->get(i);
        array->readPagedBuffers(pagedStream);
    }
}


////////////////////////////////////////


AttributeTopology::AttributeTopology()
{
}

AttributeTopology::AttributeTopology(const AttributeTopology& rhs)
    : mPrimitives(rhs.mPrimitives)
{
}


bool
AttributeTopology::operator==(const AttributeTopology& other) const
{
    if (this->size() != other.size())   return false;

    for (Index i = 0; i < this->size(); i++) {
        if (*this->primitive(i) != *other.primitive(i)) {
            return false;
        }
    }

    return true;
}


void
AttributeTopology::write(std::ostream& os) const {

    uint32_t numPrimitives(mPrimitives.size());
    os.write(reinterpret_cast<const char*>(&numPrimitives), sizeof(uint32_t));

    for (const auto& primitive : mPrimitives) {
        uint16_t vertices = static_cast<uint16_t>(primitive->vertices());
        assert(vertices > 0);
        uint32_t size = static_cast<uint32_t>(primitive->size());
        os.write(reinterpret_cast<const char*>(&vertices), sizeof(uint16_t));
        os.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));
    }

    for (const auto& primitive : mPrimitives) {
        primitive->write(os);
    }
}


void
AttributeTopology::read(std::istream& is) {

    uint32_t numPrimitives = 0;
    is.read(reinterpret_cast<char*>(&numPrimitives), sizeof(uint32_t));

    mPrimitives.reserve(numPrimitives);

    for (int i = 0; i < numPrimitives; i++) {
        uint16_t vertices = 0;
        is.read(reinterpret_cast<char*>(&vertices), sizeof(uint16_t));
        assert(vertices > 0);
        uint32_t size = 0;
        is.read(reinterpret_cast<char*>(&size), sizeof(uint32_t));
        mPrimitives.emplace_back(new Primitive(static_cast<Index>(size), static_cast<Index>(vertices)));
    }

    for (auto primitive : mPrimitives) {
        primitive->read(is);
    }
}


////////////////////////////////////////


} // namespace points
} // namespace OPENVDB_VERSION_NAME
} // namespace openvdb

// Copyright (c) 2012-2017 DreamWorks Animation LLC
// All rights reserved. This software is distributed under the
// Mozilla Public License 2.0 ( http://www.mozilla.org/MPL/2.0/ )
