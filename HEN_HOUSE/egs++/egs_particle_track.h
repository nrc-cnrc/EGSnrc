/*
###############################################################################
#
#  EGSnrc egs++ particle tracks headers
#  Copyright (C) 2015 National Research Council Canada
#
#  This file is part of EGSnrc.
#
#  EGSnrc is free software: you can redistribute it and/or modify it under
#  the terms of the GNU Affero General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
#
#  EGSnrc is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#  FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
#  more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with EGSnrc. If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################
#
#  Author:          Georgi Gerganov, 2009
#
#  Contributors:    Iwan Kawrakow
#                   Reid Townson
#
###############################################################################
*/


/*! \file egs_particle_track.h
 *  \brief EGS_ParticleTrack class header file
 *  \GG
 */

#ifndef EGS_PARTICLE_TRACK_
#define EGS_PARTICLE_TRACK_

#include "egs_vector.h"
#include "egs_config1.h"
#include "egs_functions.h"

#include <fstream>
using namespace std;

/*! \brief A class representing a single track of a particle.

  \ingroup egspp_main

  This class contains all the necessary information about a single track -
  charge and energy of the particle, coordinates of interaction points
  along the track (vertices), etc. (might add additional data in future).
  This class is not meant to be used directly in the application as it is
  used by EGS_ParticleTrackContainer.
*/
class EGS_EXPORT EGS_ParticleTrack {

public:

    /*! \brief Structure to store the data for each interaction along the
      track.
    */
    struct Vertex {
        EGS_Vector x;   //!< interaction (vertex) coordinates
        EGS_Float e;    //!< energy of the particle in this vertex

        Vertex() : x(0,0,0), e(0) {};
        Vertex(EGS_Float px, EGS_Float py, EGS_Float pz) {
            x.x = px;
            x.y = py;
            x.z = pz;
            e = 1;
        };
        Vertex(EGS_Float px, EGS_Float py, EGS_Float pz, EGS_Float pe) {
            x.x = px;
            x.y = py;
            x.z = pz;
            e = pe;
        };
        Vertex(const EGS_Vector &px, EGS_Float pe) : x(px), e(pe) {};
    };

    /*! \brief Structure describing the particle being tracked. */
    struct ParticleInfo {
        EGS_I32 q;      //!< particle charge
        ParticleInfo(EGS_I32 pq) {
            q = pq;
        };
        ~ParticleInfo() { };
    };

    /*! \brief Initialize the track and allocate minimum memory */
    EGS_ParticleTrack() : m_pInfo(NULL), m_size(16), m_nVertices(0) {
        m_track = new Vertex* [m_size];
    }

    /*! \brief The destructor. Deallocate all memory. */
    ~EGS_ParticleTrack() {
        clearTrack();
        if (m_track) {
            delete [] m_track;
        }
        m_track = NULL;
    };

    /*! \brief Deallocate particle information and vertex data.

      Practically, this method is quite the same as the Destructor, with
      the difference that it allows further track scoring if necessary.
    */
    void clearTrack();

    /*! \brief Write this track to a "track space" file.

      Writes the particle information block m_pInfo, the number of vertices
      in this track and finally all vertex data.
    */
    int writeTrack(ofstream *trsp);

    /*! \brief Add additional point of interaction (Vertex) to the track. */
    void addVertex(Vertex *x);

    /*! \brief Gets vertex number \a v .

        If \a v is less than 0 or greater than m_nVertices - 1 the return
        value is \c NULL .
    */
    Vertex *getVertex(int v);

    /*! \brief Get number of vertices currently in the track. */
    int getNumVertices() {
        return m_nVertices;
    };

    /*! \brief Define the type of the particle being tracked. */
    void setParticleInfo(ParticleInfo *p) {
        m_pInfo = p;
    };

    /*! \brief Get the type of the particle being tracked. */
    ParticleInfo *getParticleInfo() {
        return m_pInfo;
    };

protected:

    /*! \brief Resize the array containing the vertices. */
    void grow();

    ParticleInfo    *m_pInfo;       //!< type of the tracked particle

    int             m_size;         //!< current size of the vertex array
    int             m_nVertices;    //!< current number of vertices in track
    Vertex          **m_track;      //!< the array with the vertices
};

/*! \brief A class that stores all the tracks in a simulation.

  \ingroup egspp_main

  The EGS_ParticleTrackContainer class represents the place where the user
  should store everything related to particle tracks (i.e. number of
  events scored, type of the tracked particles, trajectories, etc.). This
  information can be saved in a separate file and later used for
  visualization of the simulated particles.
*/
class EGS_EXPORT EGS_ParticleTrackContainer {

public:

    /*! \brief Basic Constructor. Initializes all variables. */
    EGS_ParticleTrackContainer() : m_nEvents(0), m_nTracks(0),
        m_totalTracks(0), m_stackMap(NULL), m_isScoring(NULL), m_bufferSize(0),
        m_buffer(NULL), m_trspFile(NULL) {};

    /*! \brief Constructor.

      Prepares data to be written in a 'track space file' file called \a
      fname . Sets the m_bufferSize property of the class equal to
      \a buf_size which defines how many tracks the container will store
      before flushing them to the output file.
    */
    EGS_ParticleTrackContainer(const char *fname, int buf_size) : m_nEvents(0),
        m_nTracks(0), m_totalTracks(0), m_isScoring(NULL), m_bufferSize(0), m_buffer(0),
        m_trspFile(NULL) {
        m_bufferSize = buf_size;

        // initialize the arrays
        m_buffer = new EGS_ParticleTrack* [m_bufferSize];
        m_stackMap = new int [m_bufferSize];
        m_isScoring = new bool [m_bufferSize];
        for (int i = 0; i < m_bufferSize; ++i) {
            m_buffer[i] = new EGS_ParticleTrack();
            m_stackMap[i] = -1;
            m_isScoring[i] = false;
        }

        // open output file and write header
        m_trspFilename = string(fname);
        m_trspFile = new ofstream(m_trspFilename.c_str(), ios::binary);
        int dummy = 0;
        // at the end this will be replaced with the number of recorded tracks
        m_trspFile->write((char *)&dummy, sizeof(int));
    };

    /*! \brief The Destructor. Deallocate all allocated memory. */
    ~EGS_ParticleTrackContainer() {
        // if any particles are left in memory -> flush them to the file
        if (m_nTracks > 0) {
            flushBuffer();
        }

        if (m_buffer) {
            for (int i = 0; i < m_bufferSize; i++) {
                if (m_buffer[i]) {
                    delete m_buffer[i];
                }
                m_buffer[i] = NULL;
            }
            delete [] m_buffer;
            m_buffer = NULL;
        }
        if (m_stackMap) {
            delete [] m_stackMap;
        }
        if (m_isScoring) {
            delete [] m_isScoring;
        }
        if (m_trspFile) {
            delete m_trspFile;
        }
        m_trspFile = NULL;
    };

    /*! \brief Save the number of events (for example, decays) tracked so far. */
    void setEvents(int e) {
        m_nEvents = e;
    }

    /*! \brief Get the number of events tracked so far. */
    int getEvents() {
        return m_nEvents;
    }

    /*! \brief Get the number of vertices in the track currently being scorred */
    int getCurrentNumVertices() {
        return (m_nTracks > 0) ? m_buffer[m_nTracks-1]->getNumVertices() : 0;
    };

    /*! \brief Add a vertex to the track currently being scorred */
    void addVertex(EGS_ParticleTrack::Vertex *x);

    /*! \brief Add a vertex to the track mapped by the internal stack.

      The \a stackIndex tells the position of the desired track in the
      stack. This way, by passing different \a stackIndex arguments,
      multiple tracks could be tracked at once.
    */
    void addVertex(int stackIndex, EGS_ParticleTrack::Vertex *x);

    /*! \brief Get the \a v vertex from the \a tr track.
        \todo Have to test this method.
    */
    EGS_ParticleTrack::Vertex *getTrackVertex(int tr, int v);

    /*! \brief Are we still scoring the current particle? */
    bool isScoringParticle() {
        return m_isScoring[m_nTracks-1];
    }

    /*! \brief Are we still scoring the particle mapped by the stack? */
    bool isScoringParticle(int stackIndex) {
        if (m_stackMap[stackIndex] < 0) {
            return false;
        }
        return m_isScoring[m_stackMap[stackIndex]];
    }

    /*! \brief Stop scoring the current particle. */
    void stopScoringParticle() {
        m_isScoring[m_nTracks-1] = false;
    }

    /*! \brief Stop scoring the particle mapped by the stack. */
    void stopScoringParticle(int stackIndex) {
        if (m_stackMap[stackIndex] >= 0) {
            m_isScoring[m_stackMap[stackIndex]] = false;
        }
        m_stackMap[stackIndex] = -1;
    }

    /*! \brief Start scoring a new track. */
    void startNewTrack();

    /*! \brief Start scoring a new track and map it on the stack.

      This method allows mapping different particles with different
      integers. This way, the track being mapped to \a stackIndex can be
      further referenced by the same integer \a stackIndex , which would
      allow the tracking of multiple tracks at once.
    */
    void startNewTrack(int stackIndex);

    /*! \brief Start scoring a new track from a particle of type \a p .*/
    void startNewTrack(EGS_ParticleTrack::ParticleInfo *p);

    /*! \brief Set the type of mapped particle to \a p .*/
    void setParticleInfo(int stackIndex, EGS_ParticleTrack::ParticleInfo *p) {
        if (m_stackMap[stackIndex] >= 0) {
            m_buffer[m_stackMap[stackIndex]]->setParticleInfo(p);
        }
    }

    /*! \brief Set the type of the currently tracked particle to \a p .*/
    void setCurrentParticleInfo(EGS_ParticleTrack::ParticleInfo *p) {
        m_buffer[m_nTracks-1]->setParticleInfo(p);
    }

    /*! \brief Load particle data from the file called \a filename .*/
    int readDataFile(const char *filename);

    /*! \brief Report results from the track scoring process so far
        \todo Maybe this method should be \c virtual ?
    */
    void reportResults(bool with_header = true);

protected:

    /*! \brief Write all track data to the file.

      If no file is open for writing, tracks are just discarded.
      Only tracks that are not being scored any more are discarded! Others
      remain in memory for further scoring.
    */
    void flushBuffer();

    /*! \brief Update the output file's header.

      Usually called at the end to update the number of events scored, etc.
    */
    void updateHeader();

    int                 m_nEvents;      //!< number of events scored
    int                 m_nTracks;      //!< number of tracks currently in memory
    int                 m_totalTracks;  //!< total number of tracks registered

    int                 *m_stackMap;    /*!< the internal stack used for
                                          mapping the tracks in the tracks
                                          array  with integers */
    bool                *m_isScoring;   /*!< which of the tracks in the
                                          array are still being scored */

    int                 m_bufferSize;   //!< max number of tracks in memory
    EGS_ParticleTrack   **m_buffer;     //!< the tracks array
    ofstream            *m_trspFile;    //!< the file to which data is output
    string              m_trspFilename; //!< filename of output file
};

#endif
