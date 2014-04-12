#ifndef COLVARPROXY_VMD_H
#define COLVARPROXY_VMD_H

#include "DrawMolecule.h"

#include "colvarmodule.h"
#include "colvarproxy.h"


/// \brief Communication between colvars and VMD (implementation of
/// \link colvarproxy \endlink)
class colvarproxy_vmd : public colvarproxy {

protected:

  DrawMolecule *vmdmol;

  std::string input_prefix_str, output_prefix_str, restart_output_prefix_str;
  size_t      restart_frequency_s;
  bool        first_timestep;
  bool        system_force_requested;

  std::vector<int>          colvars_atoms;
  std::vector<size_t>       colvars_atoms_ncopies;
  std::vector<cvm::rvector> positions;
  std::vector<cvm::rvector> total_forces;
  std::vector<cvm::rvector> applied_forces;

  size_t init_atom (in const &aid);

public:

  friend class cvm::atom;

  colvarproxy_vmd();
  ~colvarproxy_vmd();

  void log (std::string const &message);
  void fatal_error (std::string const &message);
  void exit (std::string const &message);

  inline cvm::real unit_angstrom()
  {
    return 1.0;
  }

  cvm::real boltzmann()
  {
    return 0.001987191;
  }

  inline std::string input_prefix()
  {
    return input_prefix_str;
  }

  cvm::rvector position_distance (cvm::atom_pos const &pos1,
                                  cvm::atom_pos const &pos2);
  cvm::real position_dist2 (cvm::atom_pos const &pos1,
                            cvm::atom_pos const &pos2);

  void select_closest_image (cvm::atom_pos &pos,
                             cvm::atom_pos const &ref_pos);


  void load_atoms (char const *filename,
                   std::vector<cvm::atom> &atoms,
                   std::string const pdb_field,
                   double const pdb_field_value = 0.0);

  void load_coords (char const *filename,
                    std::vector<cvm::atom_pos> &pos,
                    const std::vector<int> &indices,
                    std::string const pdb_field,
                    double const pdb_field_value = 0.0);

};


inline cvm::rvector colvarproxy_vmd::position_distance (cvm::atom_pos const &pos1,
                                                         cvm::atom_pos const &pos2)
{
  Position const p1 (pos1.x, pos1.y, pos1.z);
  Position const p2 (pos2.x, pos2.y, pos2.z);
  // return p2 - p1
  Vector const d = this->lattice->delta (p2, p1);
  return cvm::rvector (d.x, d.y, d.z);
}


inline void colvarproxy_vmd::select_closest_image (cvm::atom_pos &pos,
                                                    cvm::atom_pos const &ref_pos)
{
  Position const p (pos.x, pos.y, pos.z);
  Position const rp (ref_pos.x, ref_pos.y, ref_pos.z);
  ScaledPosition const srp = this->lattice->scale (rp);
  Position const np = this->lattice->nearest (p, srp);
  pos.x = np.x;
  pos.y = np.y;
  pos.z = np.z;
}


inline cvm::real colvarproxy_vmd::position_dist2 (cvm::atom_pos const &pos1,
                                                   cvm::atom_pos const &pos2)
{
  Lattice const *l = this->lattice;
  Vector const p1 (pos1.x, pos1.y, pos1.z);
  Vector const p2 (pos2.x, pos2.y, pos2.z);
  Vector const d = l->delta (p1, p2);
  return cvm::real (d.x*d.x + d.y*d.y + d.z*d.z);
}



#endif


// Emacs
// Local Variables:
// mode: C++
// End: