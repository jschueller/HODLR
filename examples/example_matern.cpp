#include "HODLR_Matrix.hpp"
#include "HODLR.hpp"
#include <chrono>

double hodlr_get_wtime()
{
  auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
  return milliseconds * 1e-3;
}

// Taking Matern Kernel for p = 2:
// K(r) = σ^2 * (1 + sqrt(5) * r / ρ + 5/3 * (r / ρ)^2) * exp(-sqrt(5) * r / ρ)
class Kernel : public HODLR_Matrix 
{

private:
    Mat x;
    double sigma_squared, rho;

public:

    // Constructor:
    Kernel(int N, double sigma, double rho) : HODLR_Matrix(N) 
    {
        x                   = (Vec::Random(N)).real();
        this->sigma_squared = sigma * sigma;
        this->rho           = rho;
        // This is being sorted to ensure that we get
        // optimal low rank structure:
        std::sort(x.data(), x.data() + x.size());
    };
    
    dtype getMatrixEntry(int i, int j) 
    {
        double R_by_rho = fabs(x(i) - x(j)) / rho;        
        return sigma_squared * (1 + sqrt(5) * R_by_rho + 5/3 * (R_by_rho * R_by_rho)) * exp(-sqrt(5) * R_by_rho);
    }

    // Destructor:
    ~Kernel() {};
};

int main(int argc, char* argv[]) 
{
    int N, M;
    double tolerance;

    if(argc < 4)
    {
        std::cout << "All arguments weren't passed to executable!" << std::endl;
        std::cout << "Using Default Arguments:" << std::endl;
        // Size of the Matrix in consideration:
        N          = 10000;
        // Size of Matrices at leaf level:
        M          = 200;
        // Tolerance of problem
        tolerance  = pow(10, -12);
    }

    else
    {
        // Size of the Matrix in consideration:
        N          = atoi(argv[1]);
        // Size of Matrices at leaf level:
        M          = atoi(argv[2]);
        // Tolerance of problem
        tolerance  = pow(10, -atoi(argv[3]));
    }

    // Declaration of HODLR_Matrix object that abstracts data in Matrix:
    // Taking σ = 10, ρ = 5:
    Kernel* K            = new Kernel(N, 10, 5);

    std::cout << "========================= Problem Parameters =========================" << std::endl;
    std::cout << "Matrix Size                        :" << N << std::endl;
    std::cout << "Leaf Size                          :" << M << std::endl;
    std::cout << "Tolerance                          :" << tolerance << std::endl << std::endl;

    // Variables used in timing:
    double start, end;

    std::cout << "Fast method..." << std::endl;
    
    bool is_sym = true;
    bool is_pd  = true;

    // Creating a pointer to the HODLR Tree structure:
    HODLR* T = new HODLR(N, M, tolerance);

    start = hodlr_get_wtime();
    T->assemble(K, "rookPivoting", is_sym, is_pd);
    end = hodlr_get_wtime();
    std::cout << "Time for assembly in HODLR form:" << (end - start) << std::endl;

    // Random Matrix to multiply with
    Mat x = (Mat::Random(N, 1)).real();
    // Stores the result after multiplication:
    Mat y_fast, b_fast;
    
    start  = hodlr_get_wtime();
    b_fast = T->matmatProduct(x);
    end    = hodlr_get_wtime();
    
    std::cout << "Time for matrix-vector product:" << (end - start) << std::endl << std::endl;

    start = hodlr_get_wtime();
    T->factorize();
    end   = hodlr_get_wtime();
    std::cout << "Time to factorize:" << (end-start) << std::endl;

    Mat x_fast;
    start  = hodlr_get_wtime();
    x_fast = T->solve(b_fast);
    end    = hodlr_get_wtime();

    std::cout << "Time to solve:" << (end-start) << std::endl;

    if(is_sym == true && is_pd == true)
    {
        start  = hodlr_get_wtime();
        y_fast = T->symmetricFactorTransposeProduct(x);
        end    = hodlr_get_wtime();
        std::cout << "Time to calculate product of factor transpose with given vector:" << (end - start) << std::endl;
        
        start  = hodlr_get_wtime();
        b_fast = T->symmetricFactorProduct(y_fast);
        end    = hodlr_get_wtime();
        std::cout << "Time to calculate product of factor with given vector:" << (end - start) << std::endl;        
    }
        
    start = hodlr_get_wtime();
    dtype log_det_hodlr = T->logDeterminant();
    (void)log_det_hodlr;
    end = hodlr_get_wtime();
    std::cout << "Time to calculate log determinant using HODLR:" << (end-start) << std::endl;

    // Direct method:
    start = hodlr_get_wtime();
    Mat B = K->getMatrix(0, 0, N, N);
    end   = hodlr_get_wtime();

    if(is_sym == true && is_pd == true)
    {
        start = hodlr_get_wtime();
        Eigen::LLT<Mat> llt;
        llt.compute(B);
        end = hodlr_get_wtime();
        std::cout << "Time to calculate LLT Factorization:" << (end-start) << std::endl;
    }

    else
    {
        start = hodlr_get_wtime();
        Eigen::PartialPivLU<Mat> lu;
        lu.compute(B);
        end = hodlr_get_wtime();
        std::cout << "Time to calculate LU Factorization:" << (end-start) << std::endl;        
    }

    delete K;
    delete T;

    return 0;
}
