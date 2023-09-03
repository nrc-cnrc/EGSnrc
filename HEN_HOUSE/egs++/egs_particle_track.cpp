/*
###############################################################################
#
#  EGSnrc egs++ particle tracks
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

/*! \file egs_particle_track.cpp
 *  \brief EGS_ParticleTrack implementation
 *  \GG
 */


#include "egs_particle_track.h"

void EGS_ParticleTrack::grow() {
    // calculate the new size of the vertex array
    int new_size = m_size > 0 ? m_size*2 : 16;
    // not enough ?? then allocate more
    if (m_nVertices > new_size) {
        new_size = m_nVertices;
    }
    Vertex **tmp = new Vertex* [new_size];
    if (m_track) {
        for (int i = 0; i < m_nVertices; ++i) {
            tmp[i] = m_track[i];
        }
        delete [] m_track;
        m_track = NULL;
    }
    m_track = tmp;
    m_size = new_size;
}

void EGS_ParticleTrack::clearTrack() {
    if (m_pInfo) {
        delete m_pInfo;
    }
    m_pInfo = NULL;
    if (m_track)
        for (int i = 0; i < m_nVertices; i++) {
            if (m_track[i]) {
                delete m_track[i];
            }
            m_track[i] = NULL;
        }
    m_nVertices = 0;
}

int EGS_ParticleTrack::writeTrack(ofstream *trsp) {
    // no need to write the track if it has less than 2 vertices ...
    if (m_nVertices < 2) {
        return 1;
    }
    trsp->write((char *)&m_nVertices, sizeof(int));
    trsp->write((char *)m_pInfo, sizeof(ParticleInfo));
    for (int i = 0; i < m_nVertices; i++) {
        trsp->write((char *)m_track[i],sizeof(Vertex));
    }
    return 0;
}

EGS_ParticleTrack::Vertex *EGS_ParticleTrack::getVertex(int v) {
    if ((v < 0) || (v >= m_nVertices)) {
        return NULL;
    }
    return (m_track[v]);
}

void EGS_ParticleTrack::addVertex(Vertex *x) {
    m_track[m_nVertices] = x;
    m_nVertices++;
    // resize the vertex array if necessary
    if (m_nVertices >= m_size) {
        grow();
    }
}

///////////////////////// EGS_ParticleTrackContainer ////////////////////////

void EGS_ParticleTrackContainer::startNewTrack() {
    // buffer full ? flush it
    if (m_nTracks >= m_bufferSize) {
        flushBuffer();
    }
    m_nTracks++;

    // mark the track as being scored
    m_isScoring[m_nTracks-1] = true;
}

void EGS_ParticleTrackContainer::startNewTrack(int stackIndex) {
    if (m_nTracks >= m_bufferSize) {
        flushBuffer();
    }
    m_nTracks++;

    // map the track on the stack
    m_stackMap[stackIndex] = m_nTracks-1;
    m_isScoring[m_nTracks-1] = true;
}

void EGS_ParticleTrackContainer::startNewTrack(EGS_ParticleTrack::ParticleInfo *p) {
    if (m_nTracks >= m_bufferSize) {
        flushBuffer();
    }
    m_nTracks++;
    m_isScoring[m_nTracks-1] = true;
    m_buffer[m_nTracks-1]->setParticleInfo(p);
}

void EGS_ParticleTrackContainer::flushBuffer() {
    int save = 0;
    if (m_trspFile) {
        for (int i = 0; i < m_nTracks; i++) {
            // if the particle is not being scored anymore
            if (!m_isScoring[i]) {
                // output it to the file and free memory
                if (!m_buffer[i]->writeTrack(m_trspFile)) {
                    m_totalTracks++;
                }
                m_buffer[i]->clearTrack();
            }
            else {
                // still scoring it -> save the particle
                m_buffer[save] = m_buffer[i];
                m_isScoring[i] = false;
                m_isScoring[save] = true;
                for (int idx = 0; idx < m_bufferSize; idx++) {
                    if (m_stackMap[idx] == i) {
                        m_stackMap[idx] = save;
                        break;
                    }
                }
                save++;
            }
        }
        updateHeader();
    }
    m_nTracks = save;
}

void EGS_ParticleTrackContainer::updateHeader() {
    if (m_trspFile) {
        ostream::off_type pos = m_trspFile->tellp();
        m_trspFile->seekp(0,ios::beg);
        m_trspFile->write((char *) &m_totalTracks, sizeof(int));
        m_trspFile->seekp(pos,ios::beg);
    }
}

EGS_ParticleTrack::Vertex *EGS_ParticleTrackContainer::getTrackVertex(int tr, int v) {
    if ((tr < 0) || (tr >= m_nTracks)) {
        return NULL;
    }
    return (m_buffer[tr]->getVertex(v));
}

void EGS_ParticleTrackContainer::addVertex(EGS_ParticleTrack::Vertex *x) {
    if (!m_isScoring[m_nTracks-1]) {
        return;
    }
    m_buffer[m_nTracks-1]->addVertex(x);
}

void EGS_ParticleTrackContainer::addVertex(int stackIndex, EGS_ParticleTrack::Vertex *x) {
    if (m_stackMap[stackIndex] < 0) {
        return;
    }
    if (!m_isScoring[m_stackMap[stackIndex]]) {
        return;
    }
    m_buffer[m_stackMap[stackIndex]]->addVertex(x);
}

int EGS_ParticleTrackContainer::readDataFile(const char *filename) {
    const char *func_name = "EGS_ParticleTrackContainer::readDataFile()";
    ifstream *data = new ifstream(filename, ios::binary);
    if (!data || data->fail() || !data->good()) {
        egsWarning("%s: Unable to open track space file '%s'! No tracks loaded\n",
                   func_name, filename);
        return -1;
    }
    data->read((char *)&m_totalTracks, sizeof(int));
    egsInformation("%s: Reading %d tracks from '%s' ...\n", func_name, m_totalTracks, filename);
    m_nTracks = 0;
    int totalVertices = 0;
    m_bufferSize = m_totalTracks;
    m_buffer = new EGS_ParticleTrack* [m_totalTracks];
    m_stackMap = new int [m_totalTracks];
    m_isScoring = new bool [m_totalTracks];
    for (int i = 0; i < m_totalTracks; i++) {
        int nvertices;
        m_buffer[i] = new EGS_ParticleTrack();
        m_stackMap[i] = -1;
        m_isScoring[i] = false;
        EGS_ParticleTrack::ParticleInfo *pinfo = new EGS_ParticleTrack::ParticleInfo(0);
        data->read((char *)&nvertices,sizeof(int));
        totalVertices += nvertices;
        data->read((char *)pinfo,sizeof(EGS_ParticleTrack::ParticleInfo));
        startNewTrack(pinfo);
        for (int j = 0; j < nvertices; j++) {
            EGS_ParticleTrack::Vertex *v = new EGS_ParticleTrack::Vertex();
            data->read((char *)v,sizeof(EGS_ParticleTrack::Vertex));
            addVertex(v);
        }
    }
    egsInformation("%s: Tracks in memory: %d\n", func_name, m_nTracks);
    egsInformation("%s: Total Vertices  : %d\n", func_name, totalVertices);

    return 0;
}

void EGS_ParticleTrackContainer::reportResults(bool with_header) {
    flushBuffer();

    // see how many tracks are still being scored
    int scoring = -1;
    if (m_isScoring) {
        scoring = 0;
        for (int i = 0; i < m_bufferSize; i++) {
            if (m_isScoring[i]) {
                scoring++;
            }
        }
    }

    if (with_header) {
        egsInformation("\nParticle track scoring results:\n");
        egsInformation("=================================\n");
        egsInformation("   Total events scored:     %d\n", m_nEvents);
    }
    egsInformation("   Total tracks scored:     %d\n", m_totalTracks);
    egsInformation("   Still being scored:      ");
    if (scoring == -1) {
        egsInformation("Unknown!?\n");
    }
    else {
        egsInformation("%d\n", scoring);
        if (scoring > 0) {
            egsWarning("   *** There are particles still being tracked. This"
                       "   should never happen if this is the end of the simulation!");
        }
    }
    egsInformation("   Output file name:        %s\n\n", m_trspFilename.c_str());
}
