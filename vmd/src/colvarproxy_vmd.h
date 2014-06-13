// -*- c++ -*-

#ifndef COLVARPROXY_VMD_H
#define COLVARPROXY_VMD_H

#include <tcl.h>

#include "DrawMolecule.h"
#include "Timestep.h"
#include "Inform.h"

#include "colvarmodule.h"
#include "colvarproxy.h"
#include "colvartypes.h"


/// \brief Communication between colvars and VMD (implementation of
/// \link colvarproxy \endlink)
class colvarproxy_vmd : public colvarproxy {

protected:

  /// pointer to the VMD Tcl interpreter
  Tcl_Interp *vmdtcl;
  /// pointer to the VMD Tcl interpreter
  int *vmdtcl;
  /// pointer to the VMD main object
  VMDApp *vmd;
  /// VMD molecule id being used (must be provided at construction)
  int vmdmolid;
  /// pointer to VMD molecule (derived from vmdmolid)
  DrawMolecule *vmdmol;
  /// current frame (initial/default value: vmdmol->frame())
  int vmdmol_frame;
  /// output object
  Inform msgColvars;

public:


  friend class cvm::atom;

  colvarproxy_vmd (Tcl_Interp *vmdtcl, VMDApp *vmd, int molid);
  ~colvarproxy_vmd();

  void setup();

  inline cvm::real unit_angstrom()
  {
    return 1.0;
  }

  inline cvm::real boltzmann()
  {
    return 0.001987191;
  }

  inline cvm::real temperature()
  {
    // TODO define, document and implement a user method to set the value of this
    return 300.0;
  }

  inline cvm::real dt()
  {
    // TODO define, document and implement a user method to set the value of this
    return 1.0;
  }

  inline cvm::real rand_gaussian()
  {
    return vmd_random_gaussian();
  }

  inline int frame()
  {
    return vmdmol_frame;
  }

  inline int frame (int f)
  {
    if (vmdmol->get_frame (f) != NULL) {
      vmdmol_frame = f;
      return 0;
    } else {
      return -1;
    }
  }

  std::string input_prefix_str;
  std::string input_prefix()
  {
    return input_prefix_str;
  }

  inline std::string restart_output_prefix()
  {
    // note that this shouldn't be called while running VMD anyway
    return std::string ("");
  }

  std::string output_prefix_str;
  inline std::string output_prefix()
  {
    return output_prefix_str;
  }

  inline size_t restart_frequency() {
    return 0;
  }

  void add_energy (cvm::real energy);

  inline void request_system_force (bool yesno) {
    if (yesno = true)
      cvm::fatal_error("a bias requested system forces, which are undefined in VMD.");
  }

  cvm::rvector position_distance (cvm::atom_pos const &pos1,
                                  cvm::atom_pos const &pos2);
  cvm::real position_dist2 (cvm::atom_pos const &pos1,
                            cvm::atom_pos const &pos2);

  void select_closest_image (cvm::atom_pos &pos,
                             cvm::atom_pos const &ref_pos);

  void log (std::string const &message);
  void fatal_error (std::string const &message);
  void exit (std::string const &message);


  void load_atoms (char const *filename,
                   std::vector<cvm::atom> &atoms,
                   std::string const pdb_field,
                   double const pdb_field_value = 0.0);

  void load_coords (char const *filename,
                    std::vector<cvm::atom_pos> &pos,
                    const std::vector<int> &indices,
                    std::string const pdb_field,
                    double const pdb_field_value = 0.0);

  // no need to reimplement backup_file()
};


inline cvm::rvector colvarproxy_vmd::position_distance (cvm::atom_pos const &pos1,
                                                        cvm::atom_pos const &pos2)
{
  // TODO: add in the proxy constructor a check for orthonormal PBCs
  Timestep *ts = vmdmol->current();
  cvm::real const a = ts->a_length;
  cvm::real const b = ts->b_length;
  cvm::real const c = ts->c_length;
  cvm::rvector diff = (pos2 - pos1);
  while (diff.x <= -0.5*a) diff.x += a;
  while (diff.y <= -0.5*b) diff.y += b;
  while (diff.z <= -0.5*c) diff.z += c;
  while (diff.x  >  0.5*a) diff.x -= a;
  while (diff.y  >  0.5*b) diff.y -= b;
  while (diff.z  >  0.5*c) diff.z -= c;
  return diff;
}


inline void colvarproxy_vmd::select_closest_image (cvm::atom_pos &pos,
                                                   cvm::atom_pos const &ref_pos)
{
  cvm::rvector const diff = position_distance (ref_pos, pos);
  pos = ref_pos + diff;
}


inline cvm::real colvarproxy_vmd::position_dist2 (cvm::atom_pos const &pos1,
                                                  cvm::atom_pos const &pos2)
{
  cvm::rvector const d = position_distance (pos1, pos2);
  return cvm::real (d.x*d.x + d.y*d.y + d.z*d.z);
}


// copy constructor
inline cvm::atom::atom (cvm::atom const &a)
  : index (a.index), id (a.id), mass (a.mass)
{}


inline cvm::atom::~atom() 
{}

inline void cvm::atom::read_position()
{
  // read the position directly from the current timestep's memory
  // Note: no prior update should be required (unlike NAMD with GlobalMaster)
  DrawMolecule *vmdmol = ((colvarproxy_vmd *) cvm::proxy)->vmdmol;
  float *vmdpos = (vmdmol->get_frame (vmdmol_frame))->pos;
  this->pos = cvm::atom_pos (vmdpos[this->id*3+0],
                             vmdpos[this->id*3+1],
                             vmdpos[this->id*3+2]);
}


#endif

