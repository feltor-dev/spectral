#include <iostream>
#include <iomanip>
#include <sstream>
#include <omp.h>

#include "toefl/toefl.h"
#include "file/read_input.h"
#include "draw/host_window.h"
#include "dft_dft_solver.h"
#include "blueprint.h"
#include "particle_density.h"

/*
 * Reads parameters from given input file
 * Inititalizes the correct solver 
 * visualizes results directly on the screen
 */
using namespace std;
using namespace toefl;
    
unsigned N; //initialized by init function
double amp, imp_amp; //
const double slit = 2./500.; //half distance between pictures in units of width
double field_ratio;
unsigned width = 960, height = 1080; //initial window width & height
std::stringstream window_str;  //window name
std::vector<double> visual;
draw::ColorMapRedBlueExt map;
typedef DFT_DFT_Solver<2> Sol;
typedef typename Sol::Matrix_Type Mat;

void WindowResize( GLFWwindow* win, int w, int h)
{
    // map coordinates to the whole window
    double win_ratio = (double)w/(double)h;
    GLint ww = (win_ratio<field_ratio) ? w : h*field_ratio ;
    GLint hh = (win_ratio<field_ratio) ? w/field_ratio : h;
    glViewport( 0, 0, (GLsizei) ww, hh);
    width = w;
    height = h;
}

Blueprint read( char const * file)
{
    std::cout << "Reading from "<<file<<"\n";
    std::vector<double> para;
    try{ para = file::read_input( file); }
    catch (Message& m) 
    {  
        m.display(); 
        throw m;
    }
    Blueprint bp( para);
    amp = para[10];
    imp_amp = para[14];
    N = para[19];
    field_ratio = bp.boundary().lx/bp.boundary().ly;
    omp_set_num_threads( para[20]);
    //blob_width = para[21];
    std::cout<< "With "<<omp_get_max_threads()<<" threads\n";
    return bp;
}
    

// The solver has to have the getField( target) function returing M
// and the blueprint() function
template<class Solver>
void drawScene( const Solver& solver, draw::RenderHostData& rend)
{
    ParticleDensity particle( solver.getField( TL_POTENTIAL), solver.blueprint());
    const typename Solver::Matrix_Type * field;
    
    { //draw electrons
    field = &solver.getField( TL_ELECTRONS);
    visual = field->copy(); 
    map.scale() = fabs(*std::max_element(visual.begin(), visual.end()));
    rend.renderQuad( visual, field->cols(), field->rows(), map);
    window_str << scientific;
    window_str <<"ne / "<<map.scale()<<"\t";
    }

    { //draw Ions
    field = &solver.getField( TL_IONS);
    visual = field->copy();
    //upper right
    rend.renderQuad( visual, field->cols(), field->rows(), map);
    window_str <<" ni / "<<map.scale()<<"\t";
    }

    if( solver.blueprint().isEnabled( TL_IMPURITY))
    {
        field = &solver.getField( TL_IMPURITIES); 
        visual = field->copy();
        map.scale() = fabs(*std::max_element(visual.begin(), visual.end()));
        //lower left
        rend.renderQuad( visual, field->cols(), field->rows(), map);
        window_str <<" nz / "<<map.scale()<<"\t";
    }
    else
        rend.renderEmptyQuad( );

    { //draw potential
    typename Solver::Matrix_Type phi = solver.getField( TL_POTENTIAL);
    particle.laplace( phi );
    visual = phi.copy(); 
    map.scale() = fabs(*std::max_element(visual.begin(), visual.end()));
    rend.renderQuad( visual, field->cols(), field->rows(), map);
    window_str <<" phi / "<<map.scale()<<"\t";
    }
        
}

int main( int argc, char* argv[])
{
    //Parameter initialisation
    Blueprint bp_mod;
    if( argc == 1)
    {
        bp_mod = read("input.txt");
    }
    else if( argc == 2)
    {
        bp_mod = read( argv[1]);
    }
    else
    {
        cerr << "ERROR: Too many arguments!\nUsage: "<< argv[0]<<" [filename]\n";
        return -1;
    }
    const Blueprint bp = bp_mod;
    
    bp.display(cout);
    //construct solvers 
    DFT_DFT_Solver<2> solver2( bp);
    DFT_DFT_Solver<3> solver3( bp);
    if( bp.boundary().bc_x == TL_PERIODIC)
        bp_mod.boundary().bc_x = TL_DST10;

    const Algorithmic& alg = bp.algorithmic();
    Matrix<double, TL_DFT> ne{ alg.ny, alg.nx, 0.}, nz{ ne}, phi{ ne};
    // place some gaussian blobs in the field
    try{
        //init_gaussian( ne, 0.1,0.2, 10./128./field_ratio, 10./128., amp);
        //init_gaussian( ne, 0.1,0.4, 10./128./field_ratio, 10./128., -amp);
        init_gaussian( ne, 0.5,0.5, 10./128./field_ratio, 10./128., amp);
        //init_gaussian( ne, 0.1,0.8, 10./128./field_ratio, 10./128., -amp);
        //init_gaussian( ni, 0.1,0.5, 0.05/field_ratio, 0.05, amp);
        if( bp.isEnabled( TL_IMPURITY))
        {
            //init_gaussian( nz, 0.8,0.4, 0.05/field_ratio, 0.05, -imp_amp);
            init_gaussian_column( nz, 0.6, 0.05/field_ratio, imp_amp);
        }
        std::array< Matrix<double, TL_DFT>,2> arr2{{ ne, phi}};
        std::array< Matrix<double, TL_DFT>,3> arr3{{ ne, nz, phi}};
        Matrix<double, TL_DRT_DFT> ne_{ alg.ny, alg.nx, 0.}, nz_{ ne_}, phi_{ ne_};
        init_gaussian( ne_, 0.5,0.5, 0.05/field_ratio, 0.05, amp);
        //init_gaussian( ne_, 0.2,0.2, 0.05/field_ratio, 0.05, -amp);
        //init_gaussian( ne_, 0.6,0.6, 0.05/field_ratio, 0.05, -amp);
        //init_gaussian( ni_, 0.5,0.5, 0.05/field_ratio, 0.05, amp);
        if( bp.isEnabled( TL_IMPURITY))
        {
            init_gaussian( nz_, 0.5,0.5, 0.05/field_ratio, 0.05, -imp_amp);
            //init_gaussian( nz_, 0.2,0.2, 0.05/field_ratio, 0.05, -imp_amp);
            //init_gaussian( nz_, 0.6,0.6, 0.05/field_ratio, 0.05, -imp_amp);
        }
        std::array< Matrix<double, TL_DRT_DFT>,2> arr2_{{ ne_, phi_}};
        std::array< Matrix<double, TL_DRT_DFT>,3> arr3_{{ ne_, nz_, phi_}};
        //now set the field to be computed
        if( !bp.isEnabled( TL_IMPURITY))
        {
            if( bp.boundary().bc_x == TL_PERIODIC)
                solver2.init( arr2, TL_IONS);
        }
        else
        {
            if( bp.boundary().bc_x == TL_PERIODIC)
                solver3.init( arr3, TL_IONS);
        }
    }catch( Message& m){m.display();}

    ////////////////////////////////glfw//////////////////////////////
    {

    height = width/field_ratio;
    GLFWwindow* w = draw::glfwInitAndCreateWindow( width, height, "");
    draw::RenderHostData render( 2,2);

    glfwSetWindowSizeCallback( w, WindowResize);
    glfwSetInputMode(w, GLFW_STICKY_KEYS, GL_TRUE);

    double t = 3*alg.dt;
    Timer timer;
    Timer overhead;
    cout<< "HIT ESC to terminate program \n"
        << "HIT S   to stop simulation \n"
        << "HIT R   to continue simulation!\n";
    while( !glfwWindowShouldClose(w))
    {
        overhead.tic();
        //ask if simulation shall be stopped
        glfwPollEvents();
        if( glfwGetKey( w, 'S')) 
        {
            do
            {
                glfwWaitEvents();
            } while( !glfwGetKey(w,  'R') && 
                     !glfwGetKey(w,  GLFW_KEY_ESCAPE));
        }
        
        //draw scene
        if( !bp.isEnabled( TL_IMPURITY))
        {
            if( bp.boundary().bc_x == TL_PERIODIC)
                drawScene( solver2, render);
        }
        else
        {
            if( bp.boundary().bc_x == TL_PERIODIC)
                drawScene( solver3, render);
        }
        window_str << setprecision(2) << fixed;
        window_str << " &&   time = "<<t;
        glfwSetWindowTitle(w, (window_str.str()).c_str() );
        window_str.str("");
        glfwSwapBuffers( w );
#ifdef TL_DEBUG
        glfwWaitEvents();
        if( glfwGetKey( w,'N'))
        {
#endif
        timer.tic();
        for(unsigned i=0; i<N; i++)
        {
            if( !bp.isEnabled( TL_IMPURITY))
            {
                if( bp.boundary().bc_x == TL_PERIODIC)
                    solver2.step( );
            }
            else
            {
                if( bp.boundary().bc_x == TL_PERIODIC)
                    solver3.step( );
            }
            t+= alg.dt;
        }
        timer.toc();
#ifdef TL_DEBUG
            cout << "Next "<<N<<" Steps\n";
        }
#endif
        overhead.toc();
    }
    glfwTerminate();
    cout << "Average time for one step =                 "<<timer.diff()/(double)N<<"s\n";
    cout << "Overhead for visualisation, etc. per step = "<<(overhead.diff()-timer.diff())/(double)N<<"s\n";
    }
    //////////////////////////////////////////////////////////////////
    fftw_cleanup();
    return 0;

}
