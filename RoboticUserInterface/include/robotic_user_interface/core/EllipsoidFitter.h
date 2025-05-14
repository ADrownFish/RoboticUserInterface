#include <vector>
#include <Eigen/Dense>

class EllipsoidFitter {
public:
    EllipsoidFitter() = default;

    // 添加数据点
    void addPoint(double x, double y, double z) {
        points.emplace_back(x, y, z);
    }

    // 执行椭球拟合，返回是否成功
    bool fit() {
        const size_t num_points = points.size();
        if (num_points < 9) {  // 至少需要9个点求解9个参数
            return false;
        }

        // 构建最小二乘矩阵方程 M * v = b
        Eigen::MatrixXd M(num_points, 9);
        Eigen::VectorXd b(num_points);
        
        for (size_t i = 0; i < num_points; ++i) {
            const double x = points[i].x();
            const double y = points[i].y();
            const double z = points[i].z();

            M(i, 0) = x*x;        // a
            M(i, 1) = y*y;        // b
            M(i, 2) = z*z;        // c
            M(i, 3) = 2*x*y;      // d
            M(i, 4) = 2*x*z;      // e
            M(i, 5) = 2*y*z;      // f
            M(i, 6) = 2*x;        // g
            M(i, 7) = 2*y;        // h
            M(i, 8) = 2*z;        // i

            b(i) = 1.0;
        }

        // 使用SVD求解最小二乘问题
        Eigen::VectorXd v = M.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b);

        // 提取参数
        const double a = v(0);
        const double b_val = v(1);
        const double c = v(2);
        const double d = v(3);
        const double e = v(4);
        const double f = v(5);
        const double g = v(6);
        const double h = v(7);
        const double i = v(8);

        // 构造Q矩阵和P向量
        Eigen::Matrix3d Q;
        Q << a, d, e,
             d, b_val, f,
             e, f, c;

        const Eigen::Vector3d P(g, h, i);

        // 检查矩阵是否可逆
        Eigen::Matrix3d Q_inv;
        bool invertible;
        double determinant;
        Q.computeInverseAndDetWithCheck(Q_inv, determinant, invertible);

        if (!invertible) {
            return false;
        }

        // 计算中心点
        center = -Q_inv * P;
        return true;
    }

    // 获取椭球中心坐标
    Eigen::Vector3d getCenter() const {
        return center;
    }

    // 清空数据点
    void clear() {
        points.clear();
    }

private:
    std::vector<Eigen::Vector3d> points;
    Eigen::Vector3d center = Eigen::Vector3d::Zero();
};