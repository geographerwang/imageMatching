#include "Common/Log.h"
#include "Common/Image.h"
#include "Features/FeatureHandler.h"

#include <iostream>
#include <set>
#include <dirent.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

void setMovie( std::string name, std::string imagePath );
void findMovie( std::string imagePath );

void setShowMatches(bool value);
void testDir( std::string path );

Image* processImage( std::string path );
void getFileList(std::string path);
void printFileNames();

std::set< std::string > _files;
std::vector< std::pair< std::string, Image*  > > _movies;

FeatureHandler* _fH;
double _ratio;
int _nMatchesThresh;
bool _showMatches;

bool Log::debug = true;

int main( int argc, char** argv )
{
	_fH = new FeatureHandler(DET_SURF, DES_SURF, DESM_FB);
	_ratio = 0.85;
	_nMatchesThresh = 20;
	_showMatches = false;

	setMovie( "The Grey", "./data/movies/grey.jpg" );
	setMovie( "The Silence of the Lambs", "./data/movies/lambs.jpg" );
	setMovie( "The Dark Knight", "./data/movies/batman.jpg" );

	testDir( "./data/movies/tests" );
}

void setMovie( std::string name, std::string imagePath )
{
	Image* img = processImage(imagePath);

	_movies.push_back( std::make_pair< std::string, Image* >( name, img) );
}

void findMovie( std::string imagePath )
{
	Image* img = processImage(imagePath);

	bool found = false;
	std::vector< std::pair< std::string, Image*  > >::iterator it = _movies.begin();
	for( ; !found && it != _movies.end(); it++ )
	{
		_fH->findGoodMatches( img, it->second, _ratio );
		int matchThreshold = img->getGoodMatches( it->second ).size();

		if( matchThreshold >= _nMatchesThresh )
		{
			std::cout << "Found movie for image '" << imagePath << "': " << it->first
					<< ". With " << matchThreshold << " feature matches."<< std::endl;

			if(_showMatches)
			{
				cv::Mat out;
				img->drawMatches( it->second, matchThreshold, MA_GD, out, cv::Scalar(255,0,0,255) );
				cv::namedWindow("Matches", CV_WINDOW_NORMAL);
				cv::imshow("Matches", out);
				cv::waitKey(0);
			}
			found = true;
		}
	}
	if( !found ) std::cout << "Was not able to find movie for image '" << imagePath << "'." << std::endl;
}

void testDir( std::string path )
{
	getFileList(path);

	std::set< std::string >::iterator it = _files.begin();
	for( ; it != _files.end(); it++ )
	{
		findMovie( path+"/"+*it );
	}
}

Image* processImage( std::string path )
{
	Image* img = new Image( path, 0.0f, 0.0f, 0.0f, 0.0f );

	_fH->detect( img );
	_fH->computeDescriptors( img );

	return img;
}

void setShowMatches(bool value)
{
	_showMatches = value;
}

void printFileNames()
{
	for( std::set< std::string >::iterator it = _files.begin(); it != _files.end(); it++ )
	{
		std::cout << (*it) << std::endl;
	}
}

void getFileList( std::string path )
{
	DIR *dir;
	struct dirent *ent;
	if( ( dir = opendir( path.c_str() ) ) != NULL )
	{
		while( ( ent = readdir( dir ) ) != NULL)
		{
			if( ent->d_type == DT_REG )
			{
				_files.insert( std::string( ent->d_name ) );
			}
		}
		closedir (dir);
	}
	else
	{
	  Log::exit_error("Unable to access directory " + path );
	}
}
