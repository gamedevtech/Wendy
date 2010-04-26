///////////////////////////////////////////////////////////////////////
// Wendy core library
// Copyright (c) 2005 Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any
// damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any
// purpose, including commercial applications, and to alter it and
// redistribute it freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you
//     must not claim that you wrote the original software. If you use
//     this software in a product, an acknowledgment in the product
//     documentation would be appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and
//     must not be misrepresented as being the original software.
//
//  3. This notice may not be removed or altered from any source
//     distribution.
//
///////////////////////////////////////////////////////////////////////
#ifndef WENDY_MESH_H
#define WENDY_MESH_H
///////////////////////////////////////////////////////////////////////

#include <vector>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

class AABB;
class Sphere;

///////////////////////////////////////////////////////////////////////

/*! @brief Triangle mesh vertex.
 *
 *  @remarks Yes, it's insanely primitive.
 */
class MeshVertex
{
public:
  Vec3 position;
  Vec3 normal;
  Vec2 texcoord;
};

///////////////////////////////////////////////////////////////////////

class MeshEdge
{
public:
  void setIndices(unsigned int a, unsigned int b);
  unsigned int indices[2];
};

///////////////////////////////////////////////////////////////////////

class MeshTriangle
{
public:
  void setIndices(unsigned int a, unsigned int b, unsigned int c);
  void setEdges(unsigned int a, unsigned int b, unsigned int c);
  unsigned int indices[3];
  unsigned int edges[3];
  Vec3 normal;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Triangle mesh geometry.
 *
 *  A geometry is a set of triangles plus an associated shader name.
 *  Each triangle contains indices into the vertex list of the mesh.
 */
class MeshGeometry
{
public:
  typedef std::vector<MeshTriangle> TriangleList;
  TriangleList triangles;
  String shaderName;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Triangle mesh.
 *
 *  This is an ideal mesh representation intended for ease of use
 *  during calculations.  It is not intended for real-time use.
 */
class Mesh : public Resource<Mesh>
{
public:
  enum NormalType
  {
    SMOOTH_FACES,
    SEPARATE_FACES
  };
  /*! Constructor.
   *  @param name The desired name of the mesh.
   */
  Mesh(const String& name = "");
  /*! Merges neighbouring vertices according to the specified tolerance
   *  value.
   */
  void weld(float tolerance);
  /*! Merges the specified mesh with this mesh.
   */
  void merge(const Mesh& other);
  /*! Merges all the geometries in this mesh and assigns the resulting
   *  geometry the specified shader name.
   *  @remarks Duplicate vertices and triangles are not merged.
   */
  void collapseGeometries(const String& shaderName);
  /*! Returns the geometry with the specified shader name.
   */
  MeshGeometry* findGeometry(const String& shaderName);
  /*! Calculates and stores triangle and vertex normals for this
   *  mesh, according to the specified generation mode.
   */
  void calculateNormals(NormalType type = SMOOTH_FACES);
  /*! Calculates and stores triangle normals for this mesh.
   */
  void calculateTriangleNormals(void);
  /*! Calculates and stores the edges in this mesh. By default, the edge
   *  list is not created.
   */
  void calculateEdges(void);
  /*! @return @c true if this mesh is valid, otherwise @c false.
   */
  bool isValid(void) const;
  /*! @return The number of triangles in all geometries of this mesh.
   */
  unsigned int getTriangleCount(void) const;
  /*! Calculates the bounding box of this mesh.
   */
  void getBounds(AABB& bounds) const;
  /*! Calculates the bounding sphere of this mesh.
   */
  void getBounds(Sphere& bounds) const;
  typedef std::vector<MeshGeometry> GeometryList;
  typedef std::vector<MeshVertex> VertexList;
  typedef std::vector<MeshEdge> EdgeList;
  /*! The list of geometries in this mesh.
   */
  GeometryList geometries;
  /*! The list of vertices in this mesh.
   */
  VertexList vertices;
  /*! The list of edges in this mesh. By default, this is empty, but it
   *  can be calculated with Mesh::calculateEdges.
   */
  EdgeList edges;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Mesh calculation utility class.
 */
class VertexMerger
{
public:
  enum NormalMode
  {
    PRESERVE_NORMALS,
    MERGE_NORMALS,
  };
  /*! Constructor.
   */
  VertexMerger(void);
  VertexMerger(const Mesh::VertexList& vertices);
  void importPositions(const Mesh::VertexList& vertices);
  unsigned int addAttributeLayer(unsigned int vertexIndex,
                                 const Vec3& normal,
			         const Vec2& texcoord = Vec2::ZERO);
  void realizeVertices(Mesh::VertexList& result) const;
  void setNormalMode(NormalMode newMode);
private:
  struct VertexLayer
  {
    Vec3 normal;
    Vec2 texcoord;
    unsigned int index;
  };
  struct Vertex
  {
    typedef std::vector<VertexLayer> LayerList;
    Vec3 position;
    LayerList layers;
  };
  typedef std::vector<Vertex> VertexList;
  VertexList vertices;
  unsigned int targetCount;
  NormalMode mode;
};

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_MESH_H*/
///////////////////////////////////////////////////////////////////////