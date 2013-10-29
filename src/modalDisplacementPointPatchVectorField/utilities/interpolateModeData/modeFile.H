
#ifndef MODEFILE
#define MODEFILE

#include "boundBox.H"

class modeFile
{
    private:
        fileName fileN_;
        const int coordinateStartColumn_;
        const int displacementStartColumn_;
        const int skipLines_;
        const char delimiter_;
        pointField coordinates_;
        vectorField displacements_;
        std::ifstream* fpPtr_;

        /*
         * Private member functions
         */
        void readColumnData(const int startCol, vectorField& data )
        {
            data.clear();

            fpPtr_ = new std::ifstream(fileN_.c_str());
            std::ifstream& fp = *fpPtr_;
            std::string line;

            for (int i=0; i<skipLines_; i++)
            {
                getline(fp,line);
            }

            scalar x,y,z;
            while(getline(fp,line))
            {
                std::istringstream ss(line);

                scalar trash = 0;
                for (int c = 0; c < startCol; c++)
                {
                    ss >> trash;
                }
                ss >> x >> y >> z;
                data.append(vector(x,y,z));
            }
            fp.close();
        }

        void readCoordinates()
        {
            if (hasCoordinates())
            {
                Info << "Skipping read coordinates, since have them" << endl;
            }
            else
            {
                Info << "Reading coordinates from file " << fileN_ << endl;
                readColumnData(coordinateStartColumn_, coordinates_);
                Info << "Read " << coordinates().size()
                     << " coordinates from file" << endl;
            }
        }

        void readDisplacements()
        {
            Info << "Reading displacements from file " << fileN_ << endl;
            readColumnData(displacementStartColumn_, displacements_);
            Info << "Read " << displacements().size()
                 << " displacements from file" << endl;
        }

        void readDisplacements(const fileName& fileN)
        {
            changeFile(fileN);
            Info << "Reading displacements from file " << fileN_ << endl;
            readColumnData(displacementStartColumn_, displacements_);
        }



    public:
        modeFile
        (
            const fileName& fileN,
            const int coordinateStartColumn,
            const int displacementStartColumn,
            const int skipLines,
            const char delimiter
        )
        :
            fileN_(fileN),
            coordinateStartColumn_(coordinateStartColumn),
            displacementStartColumn_(displacementStartColumn),
            skipLines_(skipLines),
            delimiter_(delimiter),
            coordinates_(0),
            displacements_(0)
        {
            std::ifstream testFile(fileN_.c_str());
            if (!testFile.good())
            {
                 FatalErrorIn("modeFile::modeFile")
                     << "File not readable" << fileN_
                     << exit(FatalError);
            }
        }

        void read()
        {
            readCoordinates();
            readDisplacements();
        }

        void changeFile(const fileName& fileN)
        {
            //fpPtr_->close();
            fileN_ = fileN;
            //fpPtr_=new std::ifstream(fileN.c_str());
        }

        bool hasCoordinates()
        {
            return coordinates_.size() > 0;
        }

        /*
         * Access
         */
        
        void coordinatesInfo()
        {
            boundBox b(coordinates());
            Info << "Mode file coordinates info:\n"
                 << "\tbounding box: " <<  b << endl;
        }

        void displacementInfo()
        {
            boundBox b(displacements());
            Info << "Mode file displacements info:\n"
                 << "\t bounding box:" << b << endl;
        }

        const fileName& file() { return fileN_; };
        const pointField& coordinates() { return coordinates_; };
        const vectorField& displacements() { return displacements_; };

};
#endif
