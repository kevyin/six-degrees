using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"
#include <list>
#include <algorithm>
#include <time.h>

const char *const imdb::kActorFileName = "actors.data";
const char *const imdb::kMovieFileName = "movies.data";

imdb::imdb(const string& directory)
{
    const string actorFileName = directory + "/" + kActorFileName;
    const string movieFileName = directory + "/" + kMovieFileName;

    actorFile = acquireFileMap(actorFileName, actorInfo);
    movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const
{
    return !( (actorInfo.fd == -1) || 
            (movieInfo.fd == -1) ); 
}

// you should be implementing these two methods right here... 
bool imdb::getCredits(const string& player, vector<film>& films) const 
{ 
    vector<OffsetInt> movieOffsets = this->af_getMovieOffsets<OffsetInt>(player);

    if (!movieOffsets.empty())
    {
        // fill films vector with the movies
    	for (vector<OffsetInt>::iterator i = movieOffsets.begin(); i != movieOffsets.end(); ++i)
    	{
    		films.push_back(mf_getFilmByOffset(*i));
    	}
        return true;
    } else {
        return false;
    }
}
bool imdb::getCast(const film& movie, vector<string>& players) const 
{ 
	vector<OffsetInt> actorOffsets = this->mf_getActorOffsets<OffsetInt>(movie);

	if (!actorOffsets.empty())
	{
		// fill players vector with actors
		for (vector<OffsetInt>::iterator i = actorOffsets.begin(); i != actorOffsets.end(); ++i)
		{
			players.push_back(af_getActorNameByOffset(*i));
		}
		return true;
	} else {
		return false;
	}
}

string imdb::getRandPlayer() {
    srand(time(NULL));
    int total = af_getTotalActors();
    int ith = rand() % total + 1;
    int offset = af_getithActorOffset(ith);
    for (clock_t t = time(NULL) + 1; time(NULL) < t;){}
    
    return af_getActorNameByOffset(offset);
}

imdb::~imdb()
{
    releaseFileMap(actorInfo);
    releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM.. 
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
    struct stat stats;
    stat(fileName.c_str(), &stats);
    info.fileSize = stats.st_size;
    info.fd = open(fileName.c_str(), O_RDONLY);
    return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
    if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
    if (info.fd != -1) close(info.fd);
}

/*
 * *********************************************************************************************
 * Actor File specific functions
 * *********************************************************************************************
 */
template <typename T>
vector<T> imdb::af_getMovieOffsets(const string& player) const
{
    vector<T> movieOffsets;
    const int ithActor = this->af_findActor(player);

    if (ithActor > 0) {
        const int recordOffset = af_getithActorOffset(ithActor);
        const int nextRecordOffset = af_getithActorOffset(ithActor + 1);

        // Determine where the movie offsets start
        const int nameLength = af_getActorNameByOffset(recordOffset).length();
        const int nameShortBytes = nameLength*sizeof(char) + 1 + (nameLength % 2 == 0 ? 1 : 0) + 2; // \0 + extra padding if needed + 2 byte short
        const int byteOffset  = recordOffset + nameShortBytes + (nameShortBytes % 4 != 0 ? 2 : 0); // start record, name, short, padding

        const T* actorf_int = af_getActorFilePtrAsType<T>();
        movieOffsets = vector<T>(applyByteOffset<T>(actorf_int, byteOffset),
        						 applyByteOffset<T>(actorf_int, nextRecordOffset));
    }

    return movieOffsets;
}

int imdb::af_findActor(const string& player) const
{
    int ithActor = 1;
    string actorName;

    // bounds for binary search
    int upper_bound = af_getTotalActors();
    int lower_bound = 0;

    bool found = false;
    int move = 1; //init w/ anything non-zero
    while (found == false and move != 0)
    {
        int actorByteOffset = af_getithActorOffset(ithActor);
        actorName = af_getActorNameByOffset(actorByteOffset);
        if(actorName == player) {
        	found = true;
        	move = 0;
        } else if (actorName < player) {
			lower_bound = ithActor;
			move = (upper_bound - ithActor) / 2;
		} else {
			upper_bound = ithActor;
			move = -1*(ithActor - lower_bound) / 2;
		}
		ithActor += move;
    }
    return (found) ? ithActor : 0;
}

int imdb::af_getTotalActors() const
{
    const int32_t* actorf_int = af_getActorFilePtrAsType<int32_t>();
    return actorf_int[0];

}

int imdb::af_getithActorOffset(const int ithActor) const
{
	if (1 <= ithActor && ithActor <= af_getTotalActors()) {
		const int32_t* actorf_int = af_getActorFilePtrAsType<int32_t>();
		return actorf_int[ithActor];
	} else {
		cerr << "Warning:af_getithActorOffset: attempting to access an actor that is out of range";
		return 0;
	}
}

string imdb::af_getActorNameByOffset(const int actorByteOffset) const
{
    const int32_t* actorf_int = af_getActorFilePtrAsType<int32_t>();
    return string(reinterpret_cast<const char*>(applyByteOffset<int32_t>(actorf_int, actorByteOffset)));
}

template <typename T>
const T* imdb::af_getActorFilePtrAsType() const
{
    const T* actorf_int = static_cast<const T*>(this->actorFile);
    return actorf_int;
}

/*
 * *********************************************************************************************
 * Movie File specific functions
 * *********************************************************************************************
 */

template <typename T>
vector<T> imdb::mf_getActorOffsets(const film& movie) const
{
    vector<T> actorOffsets;
    const int ithMovie = this->mf_findMovie(movie);

    if (ithMovie > 0) {
        const int recordOffset = mf_getithMovieOffset(ithMovie);
        const int nextRecordOffset = mf_getithMovieOffset(ithMovie + 1);

        // Determine where the actor offsets start
        const int titleLength = mf_getMovieTitleByOffset(recordOffset).length();
        const int titleYearBytes = titleLength*sizeof(char) + 1 + 1; // \0 and 1 byte for year
        const int titleYearShortBytes  = titleYearBytes + (titleYearBytes % 2 != 0 ? 1 : 0) + 2; // title/Year, padding, short,
        const int byteOffset = recordOffset + titleYearShortBytes + (titleYearShortBytes % 4 != 0 ? 2 : 0); // start, title/year/short, padding

        const T* movief_int = mf_getMovieFilePtrAsType<T>();
        actorOffsets = vector<T>(applyByteOffset<T>(movief_int, byteOffset),
        						 applyByteOffset<T>(movief_int, nextRecordOffset));
    }

    return actorOffsets;
}

int imdb::mf_findMovie(const film& movie) const
{
    int ithMovie = 1; // start with first movie
    film currFilm;

    // bounds for binary search
    int upper_bound = mf_getTotalMovies();
    int lower_bound = 0;

    bool found = false;
    int move = 1; //init w/ anything non-zero
    while (found == false and move != 0)
    {
        int actorByteOffset = mf_getithMovieOffset(ithMovie);
        currFilm = mf_getFilmByOffset(actorByteOffset);
        if(currFilm == movie) {
        	found = true;
        	move = 0;
        } else if (currFilm < movie) {
            lower_bound = ithMovie;
            move = (upper_bound - ithMovie) / 2;
        } else {
            upper_bound = ithMovie;
            move = -1*(ithMovie - lower_bound) / 2;
        }
        ithMovie += move;
    }
    return (found) ? ithMovie : 0;
}

template <typename T>
const T* imdb::mf_getMovieFilePtrAsType() const
{
    const T* movief_ptr = static_cast<const T*>(this->movieFile);
    return movief_ptr;
}

string imdb::mf_getMovieTitleByOffset (const int offset) const
{
    const int32_t* movief_int = mf_getMovieFilePtrAsType<int32_t>();
    return string(reinterpret_cast<const char*>(applyByteOffset<int32_t>(movief_int, offset)));
}

int imdb::mf_getMovieYearByOffset (const int offset) const
{
	// Determine where the actor offsets start
	const int titleLength = mf_getMovieTitleByOffset(offset).length();
    const int8_t* movief_int8 = mf_getMovieFilePtrAsType<int8_t>();
    int titleBytes = offset + titleLength*sizeof(char) + 1; // + 1 more byte to account for \0

    // Read the year delta
    int8_t yearDelta = *(applyByteOffset<int8_t>(movief_int8, titleBytes));
    return yearDelta + 1900;
}

film imdb::mf_getFilmByOffset(const int offset) const
{
	film f;
	f.title = mf_getMovieTitleByOffset(offset);
	f.year  = mf_getMovieYearByOffset(offset);
	return f;
}

int imdb::mf_getTotalMovies() const
{
	const int32_t* movief_int = mf_getMovieFilePtrAsType<int32_t>();
	return movief_int[0];
}

int imdb::mf_getithMovieOffset(const int ithMovie) const
{
	if (1 <= ithMovie && ithMovie <= mf_getTotalMovies()) {
	    const int32_t* movief_int = mf_getMovieFilePtrAsType<int32_t>();
	    return movief_int[ithMovie];
	} else {
		cerr << "Warning:mf_getithMovieOffset: attempting to access a movie that is out of range";
		return 0;
	}
}

// Helper Functions

template <typename T>
const T* imdb::applyByteOffset (const T* ptr, const int bytes) const
{
    int next = bytes/sizeof(T);
    return ptr + next;
}
