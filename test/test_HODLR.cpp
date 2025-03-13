// File used in CI testing
#include "HODLR_Matrix.hpp"
#include "KDTree.hpp"
#include "HODLR.hpp"

// Derived class of HODLR_Matrix which is ultimately
// passed to the HODLR_Tree class:
class Kernel_Gaussian : public HODLR_Matrix 
{

private:
    Mat x;

public:

    // Constructor:
    explicit Kernel_Gaussian(int N, int dim) : HODLR_Matrix(N) 
    {
        x = (Mat::Random(N, dim)).real();
        // This is being sorted to ensure that we get
        // optimal low rank structure:
        getKDTreeSorted(x, 0);
    };
    
    // In this example, we are illustrating usage using
    // the gaussian kernel:
    dtype getMatrixEntry(int i, int j) 
    {
        size_t dim = x.cols();

        // Value on the diagonal:
        if(i == j)
        {
            return 100;
        }
        
        // Otherwise:
        else
        {   
            dtype R2 = 0;

            for(unsigned int k = 0; k < dim; k++) 
            {
                R2 += (x(i,k) - x(j,k)) * (x(i,k) - x(j,k));
            }

            return exp(-R2);
        }
    }

    // Destructor:
    ~Kernel_Gaussian() {};
};

class Random_Matrix : public HODLR_Matrix 
{

private:
    Mat x;

public:

    // Constructor:
    explicit Random_Matrix(int N) : HODLR_Matrix(N) 
    {
        x = (Mat::Random(N, N)).real().cwiseAbs();
        x = 0.5 * (x + x.transpose());
        x = x + N * N * Mat::Identity(N, N);
    };
    
    // In this example, we are illustrating usage using
    // the gaussian kernel:
    dtype getMatrixEntry(int i, int j) 
    {
        return x(i, j);
    }

    // Destructor:
    ~Random_Matrix() {};
};

template<class HODLR_Matrix>
void testHODLR(int N, int M, double tolerance, HODLR_Matrix* K, std::string lowrank_method, std::string image_name)
{
    // Througout, we have ensured that the error in the method is lesser than 
    // N X tolerance that was requested for ACA. It is not always necessary
    // for the error for all methods to be exactly equal to the tolerance of ACA
    // Upper bound of error is ~ N * ε
    // Random Matrix to multiply with
    Mat x = (Mat::Random(N, 1)).real();
    // Stores the result after multiplication:
    Mat y_fast, b_fast;

    // Testing fast factorization:
    bool is_sym = false;
    bool is_pd  = false;

    HODLR* T = new HODLR(N, M, tolerance);
    T->assemble(K, lowrank_method, is_sym, is_pd);
    T->printTreeDetails();
    T->plotTree(image_name);
    
    b_fast      = T->matmatProduct(x);
    Mat B       = K->getMatrix(0, 0, N, N);
    Mat b_exact = B * x;
    assert((b_fast-b_exact).norm() / (b_exact.norm()) < N * tolerance);

    T->factorize();
    Mat x_fast;
    x_fast = T->solve(b_exact);
    assert((x_fast - x).norm() / (x.norm()) < N * tolerance);

    dtype log_det;
    Eigen::PartialPivLU<Mat> lu;
    lu.compute(B);
    log_det = 0.0;
    for(int i = 0; i < lu.matrixLU().rows(); i++)
    {
        log_det += log(lu.matrixLU()(i,i));
    }

    dtype log_det_hodlr = T->logDeterminant();
    (void)log_det_hodlr;
    assert(fabs(1 - fabs(log_det_hodlr/log_det)) < N * tolerance);
    delete T;

    // Testing fast symmetric factorization:
    is_sym = true;
    is_pd  = true;
    T      = new HODLR(N, M, tolerance);

    T->assemble(K, lowrank_method, is_sym, is_pd);
    b_fast = T->matmatProduct(x);
    // Computing the relative error in the solution obtained:
    assert((b_fast-b_exact).norm() / (b_exact.norm()) < N * tolerance);

    T->factorize();
    x_fast = T->solve(b_exact);
    assert((x_fast - x).norm() / (x.norm()) < N * tolerance);

    y_fast = T->symmetricFactorTransposeProduct(x);
    b_fast = T->symmetricFactorProduct(y_fast);

    assert((b_fast - b_exact).norm() / (b_exact.norm()) < N * tolerance);

    Eigen::LLT<Mat> llt;
    llt.compute(B);
    log_det = 0.0;
    for(int i = 0; i < llt.matrixL().rows(); i++)
    {
        log_det += log(llt.matrixL()(i,i));
    }
    log_det *= 2;

    log_det_hodlr = T->logDeterminant();
    assert(fabs(1 - fabs(log_det_hodlr/log_det)) < N * tolerance);

    // Getting the symmetric factor:
    Mat W  = T->getSymmetricFactor();
    Mat Wt = W.transpose();

    assert((Wt.colPivHouseholderQr().solve(W.colPivHouseholderQr().solve(b_exact)) - x).cwiseAbs().maxCoeff() < N * tolerance);
    delete T;
}

int main(int /*argc*/, char* /*argv*/[]) 
{
    // Size of the Matrix in consideration:
    int N            = 1000;
    // Size of Matrices at leaf level:
    int M            = 100;
    // Dimensionality of the problem:
    int dim          = 1;
    // Tolerance of problem
    double tolerance = pow(10, -12);

    // Dummy check to ensure that zeros are returned when HODLR_Matrix is directly used:
    HODLR_Matrix* K_dummy = new HODLR_Matrix(N);
    assert(K_dummy->getMatrixEntry(rand() % N, rand() % N) == 0);
    delete K_dummy;

    Kernel_Gaussian* K = new Kernel_Gaussian(N, dim);
    testHODLR(N, M, tolerance, K, "rookPivoting", "gaussian_kernel.svg");
    delete K;

    // Setting lower tolerance since this is a harsh test to reproduce at higher tolerances:
    tolerance = pow(10, -7);
    Random_Matrix* K2 = new Random_Matrix(N);
    testHODLR(N, M, tolerance, K2, "rookPivoting", "random_matrix_N_1000.svg");
    testHODLR(N, M, tolerance, K2, "queenPivoting", "random_matrix_N_1000.svg");
    testHODLR(N, M, tolerance, K2, "SVD", "random_matrix_N_1000.svg");
    testHODLR(N, M, tolerance, K2, "RRQR", "random_matrix_N_1000.svg");
    // Setting 20 as the rank for all the factorizations carried out:
    testHODLR(N, M, 20, K2, "rSVD", "random_matrix_N_1000.svg");
    delete K2;

    // Trying out odd sizes:
    N = 1943;
    M = 123;
    
    Random_Matrix* K3 = new Random_Matrix(N);
    testHODLR(N, M, tolerance, K3, "rookPivoting", "random_matrix_N_1943.svg");
    delete K3;

    std::cout << "Reached End of Test File Successfully! All functions work as intended!" << std::endl;
    return 0;
}
