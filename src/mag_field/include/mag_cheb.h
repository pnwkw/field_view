#ifndef MAG_FIELD_MAG_CHEB_H
#define MAG_FIELD_MAG_CHEB_H

#include <functional>
#include <memory>
#include <string>

#include <glbinding/gl46ext/gl.h>
#include <glm/glm.hpp>

#include <glm_helper.h>

namespace mag_field {
	class mag_cheb {
	public:
        static constexpr std::size_t DIMENSIONS = 3;
        static constexpr std::size_t MAX_SOLENOID_Z_SEGMENTS = 32;
        static constexpr std::size_t MAX_SOLENOID_P_SEGMENTS = 512;
        static constexpr std::size_t MAX_SOLENOID_R_SEGMENTS = 4096;
        static constexpr std::size_t MAX_DIPOLE_Z_SEGMENTS = 128;
        static constexpr std::size_t MAX_DIPOLE_Y_SEGMENTS = 2048;
        static constexpr std::size_t MAX_DIPOLE_X_SEGMENTS = 16384;
        static constexpr std::size_t MAX_SOLENOID_PARAMETERIZATIONS = 2048;
        static constexpr std::size_t MAX_SOLENOID_ROWS = 16384;
        static constexpr std::size_t MAX_SOLENOID_COLUMNS = 65536;
        static constexpr std::size_t MAX_SOLENOID_COEFFICIENTS = 131072;
        static constexpr std::size_t MAX_DIPOLE_PARAMETERIZATIONS = 2048;
        static constexpr std::size_t MAX_DIPOLE_ROWS = 16384;
        static constexpr std::size_t MAX_DIPOLE_COLUMNS = 65536;
        static constexpr std::size_t MAX_DIPOLE_COEFFICIENTS = 262144;
        static constexpr std::size_t MAX_CHEBYSHEV_ORDER = 32;

        template <std::size_t MAX_DIM1_SEGMENTS, std::size_t MAX_DIM2_SEGMENTS, std::size_t MAX_DIM3_SEGMENTS>
        struct SegmentsUniform {
            float MinZ;
            float MaxZ;
            float MultiplicativeFactor;

            int ZSegments;

            float SegDim1[MAX_DIM1_SEGMENTS];

            int BegSegDim2[MAX_DIM1_SEGMENTS];
            int NSegDim2[MAX_DIM1_SEGMENTS];

            float SegDim2[MAX_DIM2_SEGMENTS];

            int BegSegDim3[MAX_DIM2_SEGMENTS];
            int NSegDim3[MAX_DIM2_SEGMENTS];

            float SegDim3[MAX_DIM3_SEGMENTS];

            int SegID[MAX_DIM3_SEGMENTS];
        };

        template <std::size_t MAX_PARAMETERIZATIONS, std::size_t MAX_ROWS, std::size_t MAX_COLUMNS, std::size_t MAX_COEFFICIENTS>
        struct ParametrizationUniform {
            float BOffsets[MAX_PARAMETERIZATIONS];
            float BScales[MAX_PARAMETERIZATIONS];
            float BMin[MAX_PARAMETERIZATIONS];
            float BMax[MAX_PARAMETERIZATIONS];

            int NRows[MAX_PARAMETERIZATIONS];
            int ColsAtRowOffset[MAX_PARAMETERIZATIONS];
            int CofsAtRowOffset[MAX_PARAMETERIZATIONS];

            int NColsAtRow[MAX_ROWS];
            int CofsAtColOffset[MAX_ROWS];

            int NCofsAtCol[MAX_COLUMNS];
            int AtColCoefOffset[MAX_COLUMNS];

            float Coeffs[MAX_COEFFICIENTS];
        };

        using SolenoidSegmentsUniform = SegmentsUniform<MAX_SOLENOID_Z_SEGMENTS, MAX_SOLENOID_P_SEGMENTS, MAX_SOLENOID_R_SEGMENTS>;
        using SolenoidParameterizationUniform = ParametrizationUniform<DIMENSIONS * MAX_SOLENOID_PARAMETERIZATIONS, MAX_SOLENOID_ROWS, MAX_SOLENOID_COLUMNS, MAX_SOLENOID_COEFFICIENTS>;

        using DipoleSegmentsUniform = SegmentsUniform<MAX_DIPOLE_Z_SEGMENTS, MAX_DIPOLE_Y_SEGMENTS, MAX_DIPOLE_X_SEGMENTS>;
        using DipoleParameterizationUniform = ParametrizationUniform<DIMENSIONS * MAX_DIPOLE_PARAMETERIZATIONS, MAX_DIPOLE_ROWS, MAX_DIPOLE_COLUMNS, MAX_DIPOLE_COEFFICIENTS>;

	private:
        SolenoidSegmentsUniform sol_segment{};
        SolenoidParameterizationUniform sol_params{};

        DipoleSegmentsUniform dip_segment{};
        DipoleParameterizationUniform dip_params{};

		int solSegCache;
		int dipSegCache;

        float minz;
        float maxz;

        std::size_t dipParams;

		glm::float32 cheb1DArray(glm::float32 x, const glm::float32 *arr, int ncf) const noexcept;

		glm::float32 cheb1DParams(glm::float32 x, const glm::float32 *Coeffs, int coeff_offset, int ncf) const noexcept;

		int findSolSegment(const glm::vec3 &pos) const noexcept;

		int findDipSegment(const glm::vec3 &pos) const noexcept;

		glm::vec3 mapToInternalSol(int segID, const glm::vec3 &rphiz) const noexcept;

		glm::vec3 mapToInternalDip(int segID, const glm::vec3 &pos) const noexcept;

		bool IsBetween(const glm::vec3 &sMin, const glm::vec3 &val, const glm::vec3 &sMax) const noexcept;

		bool IsInsideSol(int segID, const glm::vec3 &rphiz) const noexcept;

		bool IsInsideDip(int segID, const glm::vec3 &pos) const noexcept;

		glm::float32 Eval3DSol(int segID, int dim, const glm::vec3 &internal) const noexcept;

		glm::vec3 EvalSol(int segID, const glm::vec3 &rphiz) const noexcept;

		glm::float32 Eval3DDip(int segID, int dim, const glm::vec3 &internal) const noexcept;

		glm::vec3 EvalDip(int segID, const glm::vec3 &pos) const noexcept;

		glm::vec3 CarttoCyl(const glm::vec3 &pos) const noexcept;

		glm::vec3 CyltoCartCylB(const glm::vec3 &rphiz, const glm::vec3 &brphiz) const noexcept;

		glm::vec3 SolDipField(const glm::vec3 &pos, const bool useCache) noexcept;

		glm::vec3 MachineField(const glm::vec3 &pos) const noexcept;

	public:
		mag_cheb();

		void resetCache() {
			solSegCache = -1;
			dipSegCache = -1;
		};

		glm::vec3 Field(const glm::vec3 &pos, bool useCache = false) noexcept;

        SolenoidSegmentsUniform *getSolSegmentsPtr() { return &sol_segment; };

        DipoleSegmentsUniform *getDipSegmentsPtr() { return &dip_segment; };

        SolenoidParameterizationUniform *getSolParamsPtr() { return &sol_params; };

        DipoleParameterizationUniform *getDipParamsPtr() { return &dip_params; };
    };
}// namespace mag_field


#endif //MAG_FIELD_MAG_CHEB_H
