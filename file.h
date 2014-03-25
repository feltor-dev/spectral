#ifndef _FILE_
#define _FILE_

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <cassert>

#include "read_input.h"
#include "hdf5.h"
#include "hdf5_hl.h"


/**
 * @file 
 * Utility functions for proper scientific h5 file format
 */

/**
 * @brief Namespace containing all functions
 */
namespace file{

/**
 * @brief Create a string like F5 time name
 *
 * @param time Time
 *
 * @return string formatted like F5 
 */
std::string setTime( double time)
{
    std::stringstream title; 
    title << "t=";
    title << std::setfill('0');
    title   <<std::setw(6) <<std::right
            <<(unsigned)(floor(time))
            <<"."
            <<std::setw(6) <<std::right
            <<(unsigned)((time-floor(time))*1e6);
    return title.str();
}

/**
 * @brief Get the time from string
 *
 * @param s string created by setTime function
 *
 * @return The time in the string
 */
double getTime( std::string& s)
{
    return file::read_input( s)[1]; 
}

/**
 * @brief Get the title string containing time 
 *
 * @param file The opened file
 * @param idx The index of the dataset
 *
 * @return string containing group name
 */
std::string getName( hid_t file, unsigned idx)
{
    hsize_t length = H5Lget_name_by_idx( file, ".", H5_INDEX_NAME, H5_ITER_INC, idx, NULL, 10, H5P_DEFAULT);
    std::string name( length+1, 's');
    H5Lget_name_by_idx( file, ".", H5_INDEX_NAME, H5_ITER_INC, idx, &name[0], length+1, H5P_DEFAULT); 
        //std::cout << "Index "<<index<<" "<<name<<"\n";
    return name;
}

/**
 * @brief Get number of objects in file
 *
 * @param file opened file
 *
 * @return number of objects in file
 */
hsize_t getNumObjs( hid_t file)
{
    H5G_info_t group_info;
    H5Gget_info( file, &group_info);
    return group_info.nlinks;
}

/**
 * @brief Create a new T5 file
 *
 * A T5 file consists of
 *  - The "inputfile" as a character string i.e. a literal copy of the original input file
 *  - A number of time groups containing 3 2D-double-datasets each ordered by time
 *  - 1 "xfiles" group containing 4 1D-double-datasets for interstep data
 */
struct T5trunc
{
    /**
     * @brief Create a new T5 file overwriting existing ones
     *
     * @param name The name of the T5 file
     * @param input A literal copy of the input file
     */
    T5trunc( const std::string& name, const std::string& input ): name_( name)
    {
        hid_t file = H5Fcreate( name.data(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
        hsize_t size = input.size();
        status_ = H5LTmake_dataset_char( file, "inputfile", 1, &size, input.data());
        H5Fclose( file);
    }

    //T must provide the data() function that returns data on the host
    /**
     * @brief Write one time - group
     *
     * @tparam T Type of the data-container. Must provide the data() function returning a pointer to double on the host.
     * @param field1 The first dataset ("electrons")
     * @param field2 The second dataset ("ions")
     * @param field3 The third dataset ("potential")
     * @param time The time makes the group name
     * @param nNx dimension in x - direction (second index)
     * @param nNy dimension in y - direction (first index)
     */
    template< class T>
    void write( const T& field1, const T& field2, const T& field3, double time, unsigned nNx, unsigned nNy)
    {
        hid_t file = H5Fopen( name_.data(), H5F_ACC_RDWR, H5P_DEFAULT);
        hid_t grp = H5Gcreate( file, file::setTime( time).data(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT  );
        hsize_t dims[] = { nNy, nNx };
        status_ = H5LTmake_dataset_double( grp, "electrons", 2,  dims, field1.data());
        status_ = H5LTmake_dataset_double( grp, "ions", 2,  dims, field2.data());
        status_ = H5LTmake_dataset_double( grp, "potential", 2,  dims, field3.data());
        H5Gclose( grp);
        H5Fclose( file);
    }
    /**
     * @brief Write one time - group
     *
     * @tparam T Type of the data-container. Must provide the data() function returning a pointer to double on the host.
     * @param field1 The first dataset ("electrons")
     * @param field2 The second dataset ("ions")
     * @param field3 The third dataset ("impurities")
     * @param field4 The fourth dataset ("potential")
     * @param time The time makes the group name
     * @param nNx dimension in x - direction (second index)
     * @param nNy dimension in y - direction (first index)
     */
    template< class T>
    void write( const T& field1, const T& field2, const T& field3, const T& field4, double time, unsigned nNx, unsigned nNy)
    {
        hid_t file = H5Fopen( name_.data(), H5F_ACC_RDWR, H5P_DEFAULT);
        hid_t grp = H5Gcreate( file, file::setTime( time).data(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT  );
        hsize_t dims[] = { nNy, nNx };
        status_ = H5LTmake_dataset_double( grp, "electrons", 2,  dims, field1.data());
        status_ = H5LTmake_dataset_double( grp, "ions", 2,  dims, field2.data());
        status_ = H5LTmake_dataset_double( grp, "impurities", 2,  dims, field3.data());
        status_ = H5LTmake_dataset_double( grp, "potential", 2,  dims, field4.data());
        H5Gclose( grp);
        H5Fclose( file);
    }
    /**
     * @brief Append data to the xfiles
     *
     * @param mass Data
     * @param diffusion Data
     * @param energy Data
     * @param dissipation Data
     */
    void append( double mass, double diffusion, double energy, double dissipation)
    {
        mass_.push_back( mass);
        diffusion_.push_back( diffusion);
        energy_.push_back( energy); 
        dissipation_.push_back( dissipation);
    }
    ~T5trunc( )
    {
        //TODO: write xfiles as hyperslab datasets to prevent loss of data in case of
        // a crash. Also necessary for continuing simulations.
        hid_t file = H5Fopen( name_.data(), H5F_ACC_RDWR, H5P_DEFAULT);
        hid_t grp = H5Gcreate( file, "xfiles", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT  );
        hsize_t dims[] = { mass_.size() };
        status_ = H5LTmake_dataset_double( grp, "mass", 1,  dims, mass_.data());
        status_ = H5LTmake_dataset_double( grp, "diffusion", 1,  dims, diffusion_.data());
        status_ = H5LTmake_dataset_double( grp, "energy", 1,  dims, energy_.data());
        status_ = H5LTmake_dataset_double( grp, "dissipation", 1,  dims, dissipation_.data());
        H5Gclose( grp);
        H5Fclose( file);
    }

  private:
    herr_t status_;
    std::string name_;
    std::vector<double> mass_, diffusion_, energy_, dissipation_;
};



/**
 * @brief Create a HDF5 file with timegroups
 */
struct Probe
{
    /**
     * @brief Create a new Probe file overwriting existing ones
     *
     * @param name The name of the H5 file
     * @param input A literal copy of the input file
     */
    Probe( const std::string& name, const std::string& input ): name_( name)
    {
        file_ = H5Fcreate( name.data(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
        hsize_t size = input.size();
        status_ = H5LTmake_dataset_char( file_, "inputfile", 1, &size, input.data());
    }

    void createGroup( double time)
    {
        grp_ = H5Gcreate( file_, file::setTime( time).data(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT  );
    }
    void closeGroup(){ H5Gclose( grp_);}
    //T must provide the data() function that returns data on the host
    /**
     * @brief Write a dataset in the currently open group
     *
     * @tparam T Type of the data-container. Must provide the data() function returning a pointer to double on the host.
     * @param field The dataset ("name")
     * @param name Name of the dataset
     * @param nNx dimension in x - direction (second index)
     * @param nNy dimension in y - direction (first index)
     */
    template< class T>
    void write( const T& field1, const std::string& name, unsigned nNx, unsigned nNy)
    {
        hsize_t dims[] = { nNy, nNx };
        status_ = H5LTmake_dataset_double( grp_, name.data(), 2,  dims, field1.data());
    }
    ~Probe( )
    {
        H5Fclose( file_);
    }

  private:
    hid_t file_, grp_;
    herr_t status_;
    std::string name_;
};

/**
 * @brief Read only access to an existing T5 file
 */
struct T5rdonly
{
    /**
     * @brief Open a file and read input
     *
     * @param name Name of an existing t5 file
     * @param in Get the input-string 
     */
    T5rdonly( const std::string& name, std::string& in )
    {
        file_ = H5Fopen( name.data(), H5F_ACC_RDONLY, H5P_DEFAULT);
        std::string namep = file::getName( file_, 0);
        hsize_t size;
        status_ = H5LTget_dataset_info( file_, namep.data(), &size, NULL, NULL);
        in.resize( size);
        status_ = H5LTread_dataset_string( file_, namep.data(), &in[0]); 
    }

    /**
     * @brief Read a field at a specified index
     *
     * @param field Container
     * @param name Name of the field (electron, ions, potential or impurities)
     * @param idx Index
     */
    template <class T>
    void get_field( T& field, const char* name, unsigned idx)
    {
        assert( idx > 0); //idx 0 is the inputfile
        std::string grpName = file::getName( file_, idx);//get group name
        hid_t group = H5Gopen( file_, grpName.data(), H5P_DEFAULT);
        hsize_t size[2]; //get dataset size
        status_ = H5LTget_dataset_info( group, name, size, NULL, NULL);
        field.resize( size[0]*size[1]);
        status_ = H5LTread_dataset_double( group, name, &field[0] );
        H5Gclose( group); //close group
    }
    /**
     * @brief Get the time corresponding to an index
     *
     * The first output has index 1 the last has index get_size()
     * @param idx The index
     *
     * @return The time of the group
     */
    double get_time( unsigned idx)
    {
        std::string grpName = file::getName( file_, idx);//get group name
        return file::getTime( grpName);
    }

    /**
     * @brief Get the # of outputs in the file
     *
     * @return # of outputs
     */
    unsigned get_size() { return file::getNumObjs( file_) -2;}
    /**
     * @brief Get a dataset of the xfiles
     *
     * @param dataset Container
     * @param name Name of the dataset
     * @note dataset[(idx-1)*num_intersteps] corresponds to index idx; dataset contains (get_size()-1)*num_intersteps+1 elements
     */
    void get_xfile( std::vector<double>& dataset, const char* name)
    {
        hid_t group = H5Gopen( file_, "xfiles", H5P_DEFAULT);
        hsize_t size; //get size
        status_ = H5LTget_dataset_info( group, name, &size, NULL, NULL);
        dataset.resize( size);
        H5LTread_dataset_double( group, name, &dataset[0] );
        H5Gclose( group);
    }
    /**
     * @brief Close file
     */
    ~T5rdonly(){ H5Fclose( file_);}
  private:
    herr_t status_;
    hid_t file_;
};

} //namespace file

#endif//_FILE_
