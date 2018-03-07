//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GEOMETRICCUTUSEROBJECT_H
#define GEOMETRICCUTUSEROBJECT_H

// MOOSE includes
#include "CrackFrontPointsProvider.h"

#include "libmesh/libmesh_common.h"
#include "libmesh/libmesh.h" // libMesh::invalid_uint
#include "libmesh/elem.h"

using namespace libMesh;

class XFEM;

namespace Xfem
{
/// Data structure defining a cut on an element edge
struct CutEdge
{
  /// ID of the first node on the edge
  unsigned int _id1;
  /// ID of the second node on the edge
  unsigned int _id2;
  /// Fractional distance along the edge (from node 1 to 2) where the cut is located
  Real _distance;
  /// Local ID of this side in the host element
  unsigned int _host_side_id;
};

/**
 * Operator < for two CutEdge Objects
 * Needed to allow the use of std::set<CutEdge>
 * @param  lhs  CutEdge object on the left side of the comparison
 * @param  rhs  CutEdge object on the right side of the comparison
 * @return bool true if lhs < rhs
 */
inline bool
operator<(const CutEdge & lhs, const CutEdge & rhs)
{
  if (lhs._id1 != rhs._id1)
    return lhs._id1 < rhs._id1;
  else
    return lhs._id2 < rhs._id2;
}

/// Data structure defining a cut through a node
struct CutNode
{
  /// ID of the cut node
  unsigned int _id;
  /// Local ID of this node in the host element
  unsigned int _host_id;
};

/// Data structure defining a cut through a face
struct CutFace
{
  /// ID of the cut face
  unsigned int _face_id;
  /// IDs of all cut faces
  std::vector<unsigned int> _face_edge;
  /// Fractional distance along the cut edges where the cut is located
  std::vector<Real> _position;
};

/// Data structure describing geometrically described cut through 2D element
struct GeomMarkedElemInfo2D
{
  /// Container for data about all cut edges in this element
  std::vector<CutEdge> _elem_cut_edges;
  /// Container for data about all cut nodes in this element
  std::vector<CutNode> _elem_cut_nodes;
  /// Container for data about all cut fragments in this element
  std::vector<CutEdge> _frag_cut_edges;
  /// Container for data about all cut edges in cut fragments in this element
  std::vector<std::vector<Point>> _frag_edges;
};

/// Data structure describing geometrically described cut through 3D element
struct GeomMarkedElemInfo3D
{
  /// Container for data about all cut faces in this element
  std::vector<CutFace> _elem_cut_faces;
  /// Container for data about all faces this element's fragment
  std::vector<CutFace> _frag_cut_faces;
  /// Container for data about all cut faces in cut fragments in this element
  std::vector<std::vector<Point>> _frag_faces;
};

} // namespace Xfem

// Forward declarations
class GeometricCutUserObject;

template <>
InputParameters validParams<GeometricCutUserObject>();

class GeometricCutUserObject : public CrackFrontPointsProvider
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  GeometricCutUserObject(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void finalize() override;

  /**
   * Check to see whether a specified 2D element should be cut based on geometric
   * conditions
   * @param elem      Pointer to the libMesh element to be considered for cutting
   * @param cut_edges Data structure filled with information about edges to be cut
   * @param cut_nodes Data structure filled with information about nodes to be cut
   * @param time      Current simulation time
   * @return bool     true if element is to be cut
   */
  virtual bool cutElementByGeometry(const Elem * elem,
                                    std::vector<Xfem::CutEdge> & cut_edges,
                                    std::vector<Xfem::CutNode> & cut_nodes,
                                    Real time) const = 0;

  /**
   * Check to see whether a specified 3D element should be cut based on geometric
   * conditions
   * @param elem      Pointer to the libMesh element to be considered for cutting
   * @param cut_faces Data structure filled with information about edges to be cut
   * @param time      Current simulation time
   * @return bool     true if element is to be cut
   */
  virtual bool cutElementByGeometry(const Elem * elem,
                                    std::vector<Xfem::CutFace> & cut_faces,
                                    Real time) const = 0;

  /**
   * Check to see whether a fragment of a 2D element should be cut based on geometric conditions
   * @param frag_edges Data structure defining the current fragment to be considered
   * @param cut_edges  Data structure filled with information about fragment edges to be cut
   * @param time       Current simulation time
   * @return bool      true if fragment is to be cut
   */
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_edges,
                                     std::vector<Xfem::CutEdge> & cut_edges,
                                     Real time) const = 0;

  /**
   * Check to see whether a fragment of a 3D element should be cut based on geometric conditions
   * @param frag_faces Data structure defining the current fragment to be considered
   * @param cut_faces  Data structure filled with information about fragment faces to be cut
   * @param time       Current simulation time
   * @return bool      true if fragment is to be cut
   */
  virtual bool cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_faces,
                                     std::vector<Xfem::CutFace> & cut_faces,
                                     Real time) const = 0;

protected:
  /// Pointer to the XFEM controller object
  std::shared_ptr<XFEM> _xfem;

  ///@{Containers with information about all 2D and 3D elements marked for cutting by this object
  std::map<unsigned int, std::vector<Xfem::GeomMarkedElemInfo2D>> _marked_elems_2d;
  std::map<unsigned int, std::vector<Xfem::GeomMarkedElemInfo3D>> _marked_elems_3d;
  ///@}

  ///@{ Methods to pack/unpack the _marked_elems_2d and _marked_elems_3d data into a structure suitable for parallel communication
  void serialize(std::string & serialized_buffer);
  void deserialize(std::vector<std::string> & serialized_buffers);
  ///@}
};

#endif // GEOMETRICCUTUSEROBJECT_H
