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

#include <cppunit/extensions/HelperMacros.h>
#include <openvdb/points/AttributeTopology.h>
#include <openvdb/points/PointDataGrid.h>
#include <openvdb/openvdb.h>
#include <openvdb/Types.h>
#include <openvdb/Metadata.h>

#include <iostream>
#include <sstream>

class TestAttributeTopology: public CppUnit::TestCase
{
public:
    virtual void setUp() { openvdb::initialize(); }
    virtual void tearDown() { openvdb::uninitialize(); }

    CPPUNIT_TEST_SUITE(TestAttributeTopology);
    CPPUNIT_TEST(testAttributeSet);
    CPPUNIT_TEST(testPrimitive);
    CPPUNIT_TEST(testTopology);

    CPPUNIT_TEST_SUITE_END();

    void testAttributeSet();
    void testPrimitive();
    void testTopology();
}; // class TestAttributeTopology

CPPUNIT_TEST_SUITE_REGISTRATION(TestAttributeTopology);


////////////////////////////////////////


void
TestAttributeTopology::testAttributeSet()
{
    openvdb::points::AttributeSet attrSet;

    CPPUNIT_ASSERT(!attrSet.descriptor().isTopologyEnabled());

    attrSet.descriptor().enableTopology();

    CPPUNIT_ASSERT(attrSet.descriptor().isTopologyEnabled());

    attrSet.descriptor().disableTopology();

    CPPUNIT_ASSERT(!attrSet.descriptor().isTopologyEnabled());
}


void
TestAttributeTopology::testPrimitive()
{
    openvdb::points::Primitive triangles(/*size=*/10, /*vertices=*/3);

    CPPUNIT_ASSERT_EQUAL(triangles.size(), openvdb::Index(10));
    CPPUNIT_ASSERT_EQUAL(triangles.vertices(), openvdb::Index(3));
}


void
TestAttributeTopology::testTopology()
{
    openvdb::points::AttributeTopology topology;

    CPPUNIT_ASSERT(topology.empty());

    openvdb::points::Primitive::Ptr triangles(new openvdb::points::Primitive(/*size=*/10, /*vertices=*/3));

    topology.addPrimitive(triangles);

    CPPUNIT_ASSERT_EQUAL(topology.size(), openvdb::Index(1));
    CPPUNIT_ASSERT(*topology.primitive(0) == *triangles);

    openvdb::points::Primitive::Ptr quads(new openvdb::points::Primitive(/*size=*/15, /*vertices=*/4));

    topology.addPrimitive(quads);

    CPPUNIT_ASSERT_EQUAL(topology.size(), openvdb::Index(2));

    std::ostringstream ostr(std::ios_base::binary);
    openvdb::io::setDataCompression(ostr, openvdb::io::COMPRESS_BLOSC);

    openvdb::io::StreamMetadata::Ptr streamMetadata(new openvdb::io::StreamMetadata);
    openvdb::io::setStreamMetadataPtr(ostr, streamMetadata);

    topology.write(ostr);

    std::istringstream istr(ostr.str(), std::ios_base::binary);
    openvdb::io::setStreamMetadataPtr(istr, streamMetadata);

    openvdb::points::AttributeTopology newTopology;
    newTopology.read(istr);

    CPPUNIT_ASSERT(topology == newTopology);
}


// Copyright (c) 2012-2017 DreamWorks Animation LLC
// All rights reserved. This software is distributed under the
// Mozilla Public License 2.0 ( http://www.mozilla.org/MPL/2.0/ )
