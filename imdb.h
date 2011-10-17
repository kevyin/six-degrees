#ifndef __imdb__
#define __imdb__

#include "imdb-utils.h"
#include <string>
#include <vector>
#include <cstdlib>
using namespace std;

class imdb {
  
 public:
  
  /**
   * Constructor: imdb
   * -----------------
   * Constructs an imdb instance to layer on top of raw memory representations
   * stored in the specified directory.  The understanding is that the specified
   * directory contains binary files carefully formatted to compactly store
   * all of the information about the movies and actors relevant to an IMDB
   * application (like six-degrees).
   *
   * @param directory the name of the directory housing the formatted information backing the imdb.
   */

  imdb(const string& directory);

  /**
   * Predicate Method: good
   * ----------------------
   * Returns true if and only if the imdb opened without incident.
   * imdb::good would typically return false if:
   *
   *     1.) either one or both of the data files supporting the imdb were missing
   *     2.) the directory passed to the constructor doesn't exist.
   *     3.) the directory and files all exist, but you don't have the permission to read them.
   */

  bool good() const;

  /**
   * Method: getCredits
   * ------------------
   * Searches for an actor/actress's list of movie credits.  The list 
   * of credits is returned via the second argument, which you'll note
   * is a non-const vector<film> reference.  If the specified actor/actress
   * isn't in the database, then the films vector will be left empty.
   *
   * @param player the name of the actor or actresses being queried.
   * @param films a reference to the vector of films that should be updated
   *              with the list of the specified actor/actress's credits.
   * @return true if and only if the specified actor/actress appeared in the
   *              database, and false otherwise.
   */

  bool getCredits(const string& player, vector<film>& films) const;

  /**
   * Method: getCast
   * ---------------
   * Searches the receiving imdb for the specified film and returns the cast
   * by populating the specified vector<string> with the list of actors and actresses
   * who star in it.  If the movie doesn't exist in the database, the players vector
   * is cleared and its size left at 0.
   *
   * 
   * @param movie the film (title and year) being queried
   * @param players a reference to the vector of strings to be updated with the
   *                the list of actors and actresses starring in the specified film.
   *                If the movie doesn't exist, then the players vector would be cleared
   *                of all contents and resized to be of length 0.
   * @return true if and only if the specified movie appeared in the
   *              database, and false otherwise.
   */

  bool getCast(const film& movie, vector<string>& players) const;

  /**
   * Method: getRandPlayer
   * -----------------
   * Return the name of a random player in the database
   */

  string getRandPlayer(); 

  /**
   * Destructor: ~imdb
   * -----------------
   * Releases any resources associated with the imdb.
   * Self-explantory.
   */

  ~imdb();
  
 private:
  static const char *const kActorFileName;
  static const char *const kMovieFileName;
  const void *actorFile;
  const void *movieFile;
  

  // everything below here is complicated and needn't be touched.
  // you're free to investigate, but you're on your own.
  struct fileInfo {
    int fd;
    size_t fileSize;
    const void *fileMap;
  } actorInfo, movieInfo;
  
  static const void *acquireFileMap(const string& fileName, struct fileInfo& info);
  static void releaseFileMap(struct fileInfo& info);

  // marked as private so imdbs can't be copy constructed or reassigned.
  // if we were to allow this, we'd alias open files and accidentally close
  // files prematurely.. (do NOT implement these... since the client will
  // never call these, the code will never be needed.
  imdb(const imdb& original);
  imdb& operator=(const imdb& rhs);
  imdb& operator=(const imdb& rhs) const;

  // Common Types

  // Offsets are stored using 4 bytes
  typedef int32_t OffsetInt;

  /*
   * *********************************************************************************************
   * Actor File specific functions
   * *********************************************************************************************
   */

  /**
   * Method: af_getActorFilePtrAsType
   * ---------------
   * Return a pointer (of type T) to the Actor file
   *
   * @return a T* pointer
   */
  template <typename T>
  const T* af_getActorFilePtrAsType() const;

  /**
   * Method: af_getMovieOffsets
   * ---------------
   * Searches the actorsFile for the actor and returns
   * the list of movies represented as offsets to the movie file
   *
   * @param player the name of the actor
   * @return a vector of the offsets
   */
  template <typename T>
  vector<T> af_getMovieOffsets(const string& player) const;

  /**
   * Method: af_findActor
   * ---------------
   * Binary search of the actorFile for the actor
   *
   * @param player name of the actor
   * @return 0 if not found,
   * 		 else a index to the actor file representing the ith actor
   */
  int af_findActor(const string& player) const;

  /**
   * Method: af_getTotalActors
   * ---------------
   * Return the total number of actors
   *
   * @return total number of actors
   */
  int af_getTotalActors() const;

  /**
   * Method: af_getithActorOffset
   * ---------------
   * Given the ith actor, return the offset to the actual actor record
   *
   * @param ithActor
   * @return offset to actor record
   */
  int af_getithActorOffset(const int ithActor) const;

  /**
   * Method: af_getActorNameByOffset
   * ---------------
   * Given a byte offset to an actor record, return the actor name
   *
   * @param actorByteOffset
   * @return actor name
   */
  string af_getActorNameByOffset(const int actorByteOffset) const;

  /*
   * *********************************************************************************************
   * Movie File specific functions
   * *********************************************************************************************
   */

  /**
   * Method: mf_getMovieFilePtrAsType
   * ---------------
   * Return a pointer (of type T) to the Movie file
   *
   * @return T* pointer
   */
  template <typename T>
  const T* mf_getMovieFilePtrAsType() const;

  /**
   * Method: mf_getActorOffsets
   * ---------------
   * Given a film return a list of actors represented as offsets to the actor file
   * T represents the type (usually int32_t) an offset takes in the movie file
   *
   * @param movie film struct
   * @return vector of offsets
   */
  template <typename T>
  vector<T> mf_getActorOffsets(const film& movie) const;

  /**
   * Method: mf_findMovie
   * ---------------
   * binary search for the film in the movie file
   *
   * @param movie a film struct
   * @return 0 if not found, else an int representing the ithMovie in the file
   */
  int mf_findMovie(const film& movie) const;

  /**
   * Method: mf_getMovieTitleByOffset
   * ---------------
   * Given an offset to the movie record, return the movie's Title
   *
   * @param offset
   * @return movie title
   */
  string mf_getMovieTitleByOffset (const int offset) const;

  /**
   * Method: mf_getMovieYearByOffset
   * ---------------
   * Given an offset to the movie record, return the movie's Year
   *
   * @param offset
   * @return movie year
   */
  int mf_getMovieYearByOffset (const int offset) const;

  /**
   * Method: mf_getFilmByOffset
   * ---------------
   * Given an offset to the movie record, return the movie as a film struct
   *
   * @param offset
   * @return film struct
   */
  film mf_getFilmByOffset(const int offset) const;

  /**
   * Method: mf_getTotalMovies
   * ---------------
   * return the total number of movies in the movie file
   *
   * @return total number of movies
   */
  int mf_getTotalMovies() const;

  /**
   * Method: mf_getithMovieOffset
   * ---------------
   * Given the ith movie in the movie file,
   * return the byte offset to the actual record
   *
   * @param ithMovie
   * @return the offset to the ithMovie
   */
  int mf_getithMovieOffset(const int ithMovie) const;

  // Helper functions

  /**
   * Method: applyByteOffset
   * ---------------
   * Given a pointer of type T and a byte offset
   * return a new pointer shifted by the offset
   *
   * @param ptr original pointer of type T
   * @param bytes byte offset
   * @return pointer with offset applied
   */
  template <typename T>
  const T* applyByteOffset (const T* ptr, const int bytes) const;

};

#endif
