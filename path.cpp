#include "path.h"
#include <iostream>
using namespace std;

/**
 * All paths are designed to store the path from a specified
 * actor or actress to another actor or actress through a series
 * of movie-player connections.  A new path is always a partial
 * path because it only knows of the first player in the chain.
 * As a result, the embedded vector should be set to be empty, because
 * each entry in the vector is one leg in the path from an actor to
 * another.
 */

path::path(const string& player) : startPlayer(player) {} 
// ommission of links from init list calls the default constructor

/**
 * Simply tack on a new connection pair to the end of the links vector.
 * It ain't our business to be checking for consistency of connection, as
 * that's the resposibility of the surrounding class to decide (or at
 * least we're making it their business.
 */

void path::addConnection(const film& movie, const string& player)
{
  links.push_back(connection(movie, player));
} 

/**
 * Remove the last connection pair 
 * if there is one.
 */

void path::undoConnection()
{
  if (links.size() == 0) return;
  links.pop_back();
}

/**
 * Returns the last player (actor/actress) currently 
 * in the path.
 */

const string& path::getLastPlayer() const
{
  if (links.size() == 0) return startPlayer;
  return links.back().player;
}

/**
 * NB: Our implementation uses the path output operator
 * to produce its output. You should probably leave it well
 * alone and use it for your output, unless you are confident 
 * you can reproduce its behaviour in some other way.
 */
ostream& operator<<(ostream& os, const path& p)
{
  if (p.links.size() == 0) return os << string("[Empty path]") << endl;
  
  os << "\t" << p.startPlayer << " was in ";
  for (int i = 0; i < (int) p.links.size(); i++) {
    os << "\"" << p.links[i].movie.title << "\" (" << p.links[i].movie.year << ") with " 
       << p.links[i].player << "." << endl;
    if (i + 1 == (int) p.links.size()) break;
    os << "\t" << p.links[i].player << " was in ";
  }

  return os;
}
