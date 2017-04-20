#ifndef _TL_EQUATIONS_
#define _TL_EQUATIONS_

#include <array>
#include <complex>
#include "toefl/matrix.h"
#include "toefl/quadmat.h"
#include "blueprint.h"

namespace toefl{
   
/*! @addtogroup equations
 * @{
 */
/*! @brief Yield the coefficients for the local Poisson equation
 */
class Poisson
{
  public: 
    /*! @brief Initialize physical parameters
     *
     * @param phys Set the species parameters
     */
    Poisson(const Physical& phys):a_i(phys.a[0]), mu_i(phys.mu[0]), tau_i(phys.tau[0]),
                                  a_z(phys.a[1]), mu_z(phys.mu[1]), tau_z(phys.tau[1]) 
                                      {}
    /*! @brief Compute prefactors for ne and ni in local poisson equation
     *
     * @param phi   
     *  Contains the two prefactors in the local Poisson equation:
     *  phi[0] multiplies with ne, phi[1]  with ni
     * @param laplace 
     *  The laplacian in fourier space
     */
    void operator()( std::array<double,2>& phi, const double laplace) const;
    /*! @brief Compute prefactors for ne, ni and nz in local poisson equation
     *
     * @param phi   
     *  Contains the three prefactors in the local Poisson equation:
     *  phi[0] multiplies with ne, phi[1]  with ni, phi[2] with nz
     * @param laplace   
     *  The laplacian in fourier space
     */
    void operator()( std::array<double,3>& phi, const double laplace) const;
    /*! @brief Compute Gamma0_i
     *
     * @param laplace The laplacian in fourier space
     * @return Gamma_i
     */
    inline double gamma0_i( const double laplace) const;
    /*! @brief Compute Gamma0_z
     *
     * @param laplace The laplacian in fourier space
     * @return Gamma_z
     */
    inline double gamma0_z( const double laplace) const;
    /*! @brief Compute Gamma2_i
     *
     * @param laplace The laplacian in fourier space
     * @return Gamma2_i
     */
    /*! @brief Compute Gamma_i
     *
     * @param laplace The laplacian in fourier space
     * @return Gamma_i
     */
    inline double gamma1_i( const double laplace) const;
    /*! @brief Compute Gamma_z
     *
     * @param laplace The laplacian in fourier space
     * @return Gamma_z
     */
    inline double gamma1_z( const double laplace) const;
    /*! @brief Compute Gamma2_i
     *
     * @param laplace The laplacian in fourier space
     * @return Gamma2_i
     */
    //inline double gamma2_i( const double laplace) const;
    //inline double gamma2_z( const double laplace) const;
  private:
    const double a_i, mu_i, tau_i;
    const double a_z, mu_z, tau_z;
};

/*! @brief Yield the linear part of the local toefl equations
 *
 * \attention
 * The sine functions are not eigenfunctions of simple derivatives like e.g. dx!!
 */
class Equations
{
    typedef std::complex<double> complex;
  public:
    /*! @brief Initialize physical parameters
     *
     * @param phys Set the species parameters
     * @param mhw  (Modified Hasegawa Wakatani)
     *  Specify if coupling should be modified, i.e. subtract zonal
     *  averages
     */
    Equations( const Physical& phys, bool mhw = false):
        p( phys), mhw( mhw),
        dd(phys.d), nu(phys.nu), 
        g_e(phys.g_e), g_i(phys.g[0]), g_z(phys.g[1]),
        kappa_y(phys.kappa),
        tau_i(phys.tau[0]), tau_z(phys.tau[1])
    {}

    /*! @brief compute the linear part of the toefl equations without impurities
     *
     * @param coeff Contains the coefficients on output
     * @param laplace The value of the laplacian in fourier space
     * @param dy    The value of the y-derivative in fourier space
     * \note This way you have the freedom to use various expansion functions (e.g. sine, cosine or exponential functions) 
     */
    void operator()( QuadMat<complex,2>& coeff, const double laplace, const complex dy) const ;
    /*! @brief compute the linear part of the toefl equations with impurities
     *
     * @param coeff Contains the coefficients on output
     * @param laplace    The value of the laplacian in fourier space
     * @param dy    The value of the y-derivative in fourier space
     * \note This way you have the freedom to use various expansion functions (e.g. sine, cosine or exponential functions) 
     */
    void operator()(QuadMat<complex, 3>& coeff, const double laplace, const complex dy) const;
  private:
    const Poisson p;
    const bool mhw;
    const double dd, nu;
    const double g_e, g_i, g_z;
    const double kappa_y;
    const double tau_i, tau_z;
};
///@}

//changed equations to simple Laplacian!!!
void Equations::operator()( QuadMat<complex,2>& c, const double laplace, const complex dy) const
{
    double d = dd;
    if( mhw && dy == complex(0)) d = 0;// the mean value is a delta in fourier space
    if( laplace == 0)
    {
        c(0,0) = -d; c(0,1) = 0.;
        c(1,0) = 0;   c(1,1) = 0.;
        return;
    }
    std::array<double,2> phi;
    p(phi, laplace); //prefactors in Poisson equations (phi = phi[0]*ne + phi[1]*ni)
    const complex curv = kappa_y*dy; 
    const complex P = g_e*dy + curv + d;
    const complex Q = g_i*dy*p.gamma1_i(laplace) + curv*( p.gamma1_i(laplace) /*+ 0.5 *p.gamma2_i(laplace)*/);

    c(0,0) = P*phi[0] - curv - d - nu*pow(-laplace, 2); c(0,1) = P*phi[1];
    c(1,0) = Q*phi[0];                      c(1,1) = Q*phi[1] + tau_i*curv - nu*pow(-laplace,2);
}
void Equations::operator()( QuadMat<complex,3>& c, const double laplace, const complex dy) const
{
    double d = dd;
    if( mhw && dy == complex(0)) d = 0;// the mean value is a delta in fourier space
    if( laplace == 0)
    {
        c(0,0) = -d;  c(0,1) = 0.; c(0,2) = 0;
        c(1,0) = 0;   c(1,1) = 0.; c(1,2) = 0;
        c(2,0) = 0;   c(2,1) = 0.; c(2,2) = 0;
        return;
    }
    std::array<double,3> phi;
    p( phi, laplace);
    const complex curv = kappa_y*dy; 
    const complex P = g_e*dy + curv + d;
    const complex Q = g_i*dy*p.gamma1_i(laplace) + curv*( p.gamma1_i(laplace)/* + 0.5 *p.gamma2_i(laplace)*/);
    const complex R = g_z*dy*p.gamma1_z(laplace) + curv*( p.gamma1_z(laplace)/* + 0.5 *p.gamma2_z(laplace)*/);

    /*
    c(0,0) = P*phi[0] - curv - d + nu*pow(laplace,1); c(0,1) = P*phi[1];                       c(0,2) = P*phi[2];
    c(1,0) = Q*phi[0];                      c(1,1) = Q*phi[1] + tau_i*curv + nu*pow(laplace,1); c(1,2) = Q*phi[2];
    c(2,0) = R*phi[0];                      c(2,1) = R*phi[1];                       c(2,2) = R*phi[2] + tau_z*curv + nu*pow(laplace,1);
    */
    c(0,0) = P*phi[0] - curv - d - nu*pow(-laplace,2); c(0,1) = P*phi[1];                       c(0,2) = P*phi[2];
    c(1,0) = Q*phi[0];                      c(1,1) = Q*phi[1] + tau_i*curv - nu*pow(-laplace,2); c(1,2) = Q*phi[2];
    c(2,0) = R*phi[0];                      c(2,1) = R*phi[1];                       c(2,2) = R*phi[2] + tau_z*curv - nu*pow(-laplace,2);
}

void Poisson::operator()( std::array<double,2>& c, const double laplace) const
{
#ifdef TL_DEBUG
    if( laplace == 0)
        throw Message( "Laplace is zero in Poisson equation!", _ping_);
#endif
    //double rho = a_i*mu_i*laplace/(1.- tau_i*mu_i*laplace);
    double rho = a_i*mu_i*laplace;
    c[0] = 1./rho;
    c[1] = -a_i*gamma1_i(laplace)/rho;
}
void Poisson::operator()( std::array<double, 3>& c, const double laplace) const
{
#ifdef TL_DEBUG
    if( laplace == 0)
        throw Message( "Laplace is zero in Poisson equation!", _ping_);
#endif
    //double rho = (a_i*mu_i/(1.- tau_i*mu_i*laplace) + a_z*mu_z/(1.-tau_z*mu_z*laplace))*laplace;
    double rho = (a_i*mu_i + a_z*mu_z)*laplace;
    c[0] = 1./rho;
    c[1] = -a_i*gamma1_i(laplace)/rho;
    c[2] = -a_z*gamma1_z(laplace)/rho;
}
double Poisson::gamma0_i( const double laplace) const
{
    return (1./(1. - tau_i*mu_i*laplace));
}
double Poisson::gamma0_z( const double laplace) const
{
    return (1./(1. - tau_z*mu_z*laplace));
}
double Poisson::gamma1_i( const double laplace) const
{
    return (1./(1. - 0.5*tau_i*mu_i*laplace));
}
double Poisson::gamma1_z( const double laplace) const
{
    return (1./(1. - 0.5*tau_z*mu_z*laplace));
}
//double Poisson::gamma2_i( const double laplace) const
//{
//    const double gamma = gamma1_i(laplace);
//    return 0.5*tau_i*mu_i*laplace*gamma*gamma;
//}
//double Poisson::gamma2_z( const double laplace) const
//{
//    const double gamma = gamma1_z(laplace);
//    return 0.5*tau_z*mu_z*laplace*gamma*gamma;
//}

} //namespace toefl



#endif //_TL_EQUATIONS_
