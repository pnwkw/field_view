#include "mag_cheb.h"

#include <fstream>
#include <spdlog/spdlog.h>

#include <config.h>

template <std::size_t MAX_DIM1_SEGMENTS, std::size_t MAX_DIM2_SEGMENTS, std::size_t MAX_DIM3_SEGMENTS>
std::tuple<std::size_t, std::size_t, std::size_t> loadSegments(std::ifstream& file, mag_field::mag_cheb::SegmentsUniform<MAX_DIM1_SEGMENTS, MAX_DIM2_SEGMENTS, MAX_DIM3_SEGMENTS>& segments)
{
    file.read(reinterpret_cast<char*>(&segments.MinZ), sizeof(segments.MinZ));
    file.read(reinterpret_cast<char*>(&segments.MaxZ), sizeof(segments.MaxZ));
    file.read(reinterpret_cast<char*>(&segments.MultiplicativeFactor), sizeof(segments.MultiplicativeFactor));

    std::int32_t NSegDim1, NSegDim2, NSegDim3;

    file.read(reinterpret_cast<char*>(&NSegDim1), sizeof(std::int32_t));
    assert(NSegDim1 <= MAX_DIM1_SEGMENTS);

    segments.ZSegments = NSegDim1;

    file.read(reinterpret_cast<char*>(segments.SegDim1), NSegDim1 * sizeof(segments.SegDim1[0]));

    file.read(reinterpret_cast<char*>(segments.BegSegDim2), NSegDim1 * sizeof(segments.BegSegDim2[0]));
    file.read(reinterpret_cast<char*>(segments.NSegDim2), NSegDim1 * sizeof(segments.NSegDim2[0]));

    file.read(reinterpret_cast<char*>(&NSegDim2), sizeof(std::int32_t));
    assert(NSegDim2 <= MAX_DIM2_SEGMENTS);

    file.read(reinterpret_cast<char*>(segments.SegDim2), NSegDim2 * sizeof(segments.SegDim2[0]));

    file.read(reinterpret_cast<char*>(segments.BegSegDim3), NSegDim2 * sizeof(segments.BegSegDim3[0]));
    file.read(reinterpret_cast<char*>(segments.NSegDim3), NSegDim2 * sizeof(segments.NSegDim3[0]));

    file.read(reinterpret_cast<char*>(&NSegDim3), sizeof(std::int32_t));
    assert(NSegDim3 <= MAX_DIM3_SEGMENTS);

    file.read(reinterpret_cast<char*>(segments.SegDim3), NSegDim3 * sizeof(segments.SegDim3[0]));
    file.read(reinterpret_cast<char*>(segments.SegID), NSegDim3 * sizeof(segments.SegID[0]));

    return std::make_tuple(NSegDim1, NSegDim2, NSegDim3);
}

template <std::size_t DIMENSIONS, std::size_t MAX_PARAMETERIZATIONS, std::size_t MAX_ROWS, std::size_t MAX_COLUMNS, std::size_t MAX_COEFFICIENTS>
std::tuple<std::size_t, std::size_t, std::size_t, std::size_t> loadParams(std::ifstream& file, mag_field::mag_cheb::ParametrizationUniform<MAX_PARAMETERIZATIONS, MAX_ROWS, MAX_COLUMNS, MAX_COEFFICIENTS>& parametrizations)
{
    std::int32_t NParams, NRows, NColumns, NCoefficients;
    file.read(reinterpret_cast<char*>(&NParams), sizeof(std::int32_t));
    assert(NParams <= (MAX_PARAMETERIZATIONS / DIMENSIONS));

    file.read(reinterpret_cast<char*>(parametrizations.BOffsets), DIMENSIONS * NParams * sizeof(parametrizations.BOffsets[0]));
    file.read(reinterpret_cast<char*>(parametrizations.BScales), DIMENSIONS * NParams * sizeof(parametrizations.BScales[0]));
    file.read(reinterpret_cast<char*>(parametrizations.BMin), DIMENSIONS * NParams * sizeof(parametrizations.BMin[0]));
    file.read(reinterpret_cast<char*>(parametrizations.BMax), DIMENSIONS * NParams * sizeof(parametrizations.BMax[0]));

    file.read(reinterpret_cast<char*>(parametrizations.NRows), DIMENSIONS * NParams * sizeof(parametrizations.NRows[0]));
    file.read(reinterpret_cast<char*>(parametrizations.ColsAtRowOffset), DIMENSIONS * NParams * sizeof(parametrizations.ColsAtRowOffset[0]));
    file.read(reinterpret_cast<char*>(parametrizations.CofsAtRowOffset), DIMENSIONS * NParams * sizeof(parametrizations.CofsAtRowOffset[0]));

    file.read(reinterpret_cast<char*>(&NRows), sizeof(std::int32_t));
    assert(NRows <= MAX_ROWS);

    file.read(reinterpret_cast<char*>(parametrizations.NColsAtRow), NRows * sizeof(parametrizations.NColsAtRow[0]));
    file.read(reinterpret_cast<char*>(parametrizations.CofsAtColOffset), NRows * sizeof(parametrizations.CofsAtColOffset[0]));

    file.read(reinterpret_cast<char*>(&NColumns), sizeof(std::int32_t));
    assert(NColumns <= MAX_COLUMNS);

    file.read(reinterpret_cast<char*>(parametrizations.NCofsAtCol), NColumns * sizeof(parametrizations.NCofsAtCol[0]));
    file.read(reinterpret_cast<char*>(parametrizations.AtColCoefOffset), NColumns * sizeof(parametrizations.AtColCoefOffset[0]));

    file.read(reinterpret_cast<char*>(&NCoefficients), sizeof(std::int32_t));
    assert(NCoefficients <= MAX_COEFFICIENTS);

    file.read(reinterpret_cast<char*>(parametrizations.Coeffs), NCoefficients * sizeof(parametrizations.Coeffs[0]));

    return std::make_tuple(NParams, NRows, NColumns, NCoefficients);
}

mag_field::mag_cheb::mag_cheb() {
    std::ifstream file;
    file.open("data/field.uniform", std::ifstream::binary);

    if (!file.good()) {
        spdlog::error("Cannot load field file \"field.uniform\" from the working directory.");
        throw std::runtime_error("Cannot load filed.uniform from the working directory");
    }

    const auto [SolSegDim1, SolSegDim2, SolSegDim3] = loadSegments(file, sol_segment);
    const auto [DipSegDim1, DipSegDim2, DipSegDim3] = loadSegments(file, dip_segment);
    const auto [SParams, SRows, SCols, SCoeffs] = loadParams<DIMENSIONS>(file, sol_params);
    const auto [DParams, DRows, DCols, DCoeffs] = loadParams<DIMENSIONS>(file, dip_params);

    minz = dip_segment.MinZ;
    maxz = sol_segment.MaxZ;

    dipParams = DParams;
}

glm::vec3 mag_field::mag_cheb::Field(const glm::vec3 &pos, const bool useCache) noexcept {
	if (pos.z > minz && pos.z < maxz) {
		return SolDipField(pos, useCache);
	}
	return MachineField(pos);
}

glm::vec3 mag_field::mag_cheb::MachineField(const glm::vec3 &pos) const noexcept {
	return glm::vec3(0);
}

glm::vec3 mag_field::mag_cheb::SolDipField(const glm::vec3 &pos, const bool useCache) noexcept {
	if (pos.z > mag_cheb::sol_segment.MinZ) {
		glm::vec3 rphiz = CarttoCyl(pos);

		if (useCache) {
			if (solSegCache >= 0 && IsInsideSol(solSegCache, rphiz)) {
				glm::vec3 brphiz = EvalSol(solSegCache, rphiz);
				return CyltoCartCylB(rphiz, brphiz);
			}
		}

		int segID = findSolSegment(rphiz);
		if (segID >= 0 && IsInsideSol(segID, rphiz)) {
			glm::vec3 brphiz = EvalSol(segID, rphiz);

			if (useCache) {
				solSegCache = segID;
			}

			return CyltoCartCylB(rphiz, brphiz);
		}
	}

	if (useCache) {
		if (dipSegCache >= 0 && IsInsideDip(dipSegCache, pos)) {
			return EvalDip(dipSegCache, pos);
		}
	}

	int segID = findDipSegment(pos);
	if (segID >= 0 && IsInsideDip(segID, pos)) {

		if (useCache) {
			dipSegCache = segID;
		}

		return EvalDip(segID, pos);
	}

	return glm::vec3(0);
}

glm::vec3 mag_field::mag_cheb::CarttoCyl(const glm::vec3 &pos) const noexcept {
	return glm::vec3(glm::length(glm::vec2(pos.x, pos.y)), glm::atan(pos.y, pos.x), pos.z);
}

glm::vec3 mag_field::mag_cheb::CyltoCartCylB(const glm::vec3 &rphiz, const glm::vec3 &brphiz) const noexcept {
	const float btr = glm::length(glm::vec2(brphiz.x, brphiz.y));
	const float psiPLUSphi = glm::atan(brphiz.y, brphiz.x) + rphiz.y;

	return glm::vec3(btr * glm::cos(psiPLUSphi), btr * glm::sin(psiPLUSphi), brphiz.z);
}

glm::vec3 mag_field::mag_cheb::EvalDip(int segID, const glm::vec3 &pos) const noexcept {
	const glm::vec3 internal = mapToInternalDip(segID, pos);
	return glm::vec3(Eval3DDip(segID, 0, internal), Eval3DDip(segID, 1, internal), Eval3DDip(segID, 2, internal));
}

float mag_field::mag_cheb::Eval3DDip(int segID, int dim, const glm::vec3 &internal) const noexcept {
	const int index = DIMENSIONS * segID;
	const int n_rows = dip_params.NRows[index + dim];
    const int cols_at_row_offset = dip_params.ColsAtRowOffset[index+dim];
    const int coeffs_at_row_offset = dip_params.CofsAtRowOffset[index+dim];

	glm::float32 tmpCfs1[MAX_CHEBYSHEV_ORDER];
	glm::float32 tmpCfs0[MAX_CHEBYSHEV_ORDER];

	for (int row = 0; row < n_rows; row++) {
        const int n_cols = dip_params.NColsAtRow[cols_at_row_offset+row];
        const int coeff_at_col_offset = dip_params.CofsAtColOffset[cols_at_row_offset+row];

		for (int col = 0; col < n_cols; col++) {
            const int n_coeffs = dip_params.NCofsAtCol[coeff_at_col_offset+col];
            const int per_col_coeff_offset = dip_params.AtColCoefOffset[coeff_at_col_offset+col];

            const int coeffs_offset = coeffs_at_row_offset + per_col_coeff_offset;

			tmpCfs1[col] = cheb1DParams(internal.z, dip_params.Coeffs, coeffs_offset, n_coeffs);
		}
		tmpCfs0[row] = cheb1DArray(internal.y, tmpCfs1, n_cols);
	}

	return cheb1DArray(internal.x, tmpCfs0, n_rows);
}

glm::vec3 mag_field::mag_cheb::EvalSol(int segID, const glm::vec3 &rphiz) const noexcept {
	const glm::vec3 internal = mapToInternalSol(segID, rphiz);
	return glm::vec3(Eval3DSol(segID, 0, internal), Eval3DSol(segID, 1, internal), Eval3DSol(segID, 2, internal));
}

glm::float32 mag_field::mag_cheb::Eval3DSol(int segID, int dim, const glm::vec3 &internal) const noexcept {
	const int index = DIMENSIONS * segID;
	const int n_rows = sol_params.NRows[index + dim];
    const int cols_at_row_offset = sol_params.ColsAtRowOffset[index+dim];
    const int coeffs_at_row_offset = sol_params.CofsAtRowOffset[index+dim];

	glm::float32 tmpCfs1[MAX_CHEBYSHEV_ORDER];
	glm::float32 tmpCfs0[MAX_CHEBYSHEV_ORDER];

	for (int row = 0; row < n_rows; row++) {
        const int n_cols = sol_params.NColsAtRow[cols_at_row_offset+row];
        const int coeff_at_col_offset = sol_params.CofsAtColOffset[cols_at_row_offset+row];

		for (int col = 0; col < n_cols; col++) {
            const int n_coeffs = sol_params.NCofsAtCol[coeff_at_col_offset+col];
            const int per_col_coeff_offset = sol_params.AtColCoefOffset[coeff_at_col_offset+col];

            const int coeffs_offset = coeffs_at_row_offset + per_col_coeff_offset;

			tmpCfs1[col] = cheb1DParams(internal.z, sol_params.Coeffs, coeffs_offset, n_coeffs);
		}
		tmpCfs0[row] = cheb1DArray(internal.y, tmpCfs1, n_cols);
	}

	return cheb1DArray(internal.x, tmpCfs0, n_rows);
}

bool mag_field::mag_cheb::IsInsideDip(int segID, const glm::vec3 &pos) const noexcept {
	const int index = DIMENSIONS * segID;

	const glm::vec3 seg_min = glm::vec3(dip_params.BMin[index + 0], dip_params.BMin[index + 1], dip_params.BMin[index + 2]);
	const glm::vec3 seg_max = glm::vec3(dip_params.BMax[index + 0], dip_params.BMax[index + 1], dip_params.BMax[index + 2]);

	return IsBetween(seg_min, pos, seg_max);
}

bool mag_field::mag_cheb::IsInsideSol(int segID, const glm::vec3 &rphiz) const noexcept {
	const int index = DIMENSIONS * segID;

	const glm::vec3 seg_min = glm::vec3(sol_params.BMin[index + 0], sol_params.BMin[index + 1], sol_params.BMin[index + 2]);
	const glm::vec3 seg_max = glm::vec3(sol_params.BMax[index + 0], sol_params.BMax[index + 1], sol_params.BMax[index + 2]);

	return IsBetween(seg_min, rphiz, seg_max);
}

bool mag_field::mag_cheb::IsBetween(const glm::vec3 &sMin, const glm::vec3 &val, const glm::vec3 &sMax) const noexcept {
	return glm::all(glm::lessThanEqual(sMin, val)) && glm::all(glm::lessThanEqual(val, sMax));
}

glm::float32 mag_field::mag_cheb::cheb1DParams(glm::float32 x, const glm::float32 *Coeffs, int coeff_offset, int ncf) const noexcept {
	if (ncf <= 0)
		return 0.0f;

	float b0 = Coeffs[coeff_offset + (--ncf)], b1 = 0, b2 = 0, x2 = x + x;
	--ncf;

	for (int i = ncf; i >= 0; i--) {
		b2 = b1;
		b1 = b0;
		b0 = Coeffs[coeff_offset + i] + x2 * b1 - b2;
	}
	return b0 - x * b1;
}

glm::float32 mag_field::mag_cheb::cheb1DArray(glm::float32 x, const glm::float32 *arr, int ncf) const noexcept {
	if (ncf <= 0)
		return 0.0f;

	const float x2 = 2 * x;

	glm::vec3 b = glm::vec3(arr[--ncf], 0, 0);
	--ncf;

	const glm::vec3 t1 = glm::vec3(1, x2, -1);

	for (int i = ncf; i >= 0; i--) {
		b.z = b.y;
		b.y = b.x;
		b.x = arr[i];
		b.x = dot(t1, b);
	}

	const glm::vec3 t = glm::vec3(1, -x, 0);
	return glm::dot(t, b);
}

glm::vec3 mag_field::mag_cheb::mapToInternalDip(int segID, const glm::vec3 &pos) const noexcept {
	const int index = DIMENSIONS * segID;
	const glm::vec3 offsets = glm::vec3(dip_params.BOffsets[index + 0], dip_params.BOffsets[index + 1], dip_params.BOffsets[index + 2]);
	const glm::vec3 scales = glm::vec3(dip_params.BScales[index + 0], dip_params.BScales[index + 1], dip_params.BScales[index + 2]);

	return (pos - offsets) * scales;
}

glm::vec3 mag_field::mag_cheb::mapToInternalSol(int segID, const glm::vec3 &rphiz) const noexcept {
	const int index = DIMENSIONS * segID;
	const glm::vec3 offsets = glm::vec3(sol_params.BOffsets[index + 0], sol_params.BOffsets[index + 1], sol_params.BOffsets[index + 2]);
	const glm::vec3 scales = glm::vec3(sol_params.BScales[index + 0], sol_params.BScales[index + 1], sol_params.BScales[index + 2]);

	return (rphiz - offsets) * scales;
}

int mag_field::mag_cheb::findDipSegment(const glm::vec3 &pos) const noexcept {
	int xid, yid, zid;

	for (zid = 0; zid < dip_segment.ZSegments; zid++)
		if (pos.z < dip_segment.SegDim1[zid]) break;
	if (--zid < 0) zid = 0;

	int ysegBeg = dip_segment.BegSegDim2[zid];
	for (yid = 0; yid < dip_segment.NSegDim2[zid]; yid++)
		if (pos.y < dip_segment.SegDim2[ysegBeg + yid]) break;
	if (--yid < 0) yid = 0;
	yid += ysegBeg;

	int xsegBeg = dip_segment.BegSegDim3[yid];
	for (xid = 0; xid < dip_segment.NSegDim3[yid]; xid++)
		if (pos.x < dip_segment.SegDim3[xsegBeg + xid]) break;
	if (--xid < 0) xid = 0;
	xid += xsegBeg;

	return dip_segment.SegID[xid];
}

int mag_field::mag_cheb::findSolSegment(const glm::vec3 &rphiz) const noexcept {
	int rid, pid, zid;

	for (zid = 0; zid < sol_segment.ZSegments; zid++)
		if (rphiz.z < sol_segment.SegDim1[zid]) break;
	if (--zid < 0) zid = 0;

	int psegBeg = sol_segment.BegSegDim2[zid];
	for (pid = 0; pid < sol_segment.NSegDim2[zid]; pid++)
		if (rphiz.y < sol_segment.SegDim2[psegBeg + pid]) break;
	if (--pid < 0) pid = 0;
	pid += psegBeg;

	int rsegBeg = sol_segment.BegSegDim3[pid];
	for (rid = 0; rid < sol_segment.NSegDim3[pid]; rid++)
		if (rphiz.x < sol_segment.SegDim3[rsegBeg + rid]) break;
	if (--rid < 0) rid = 0;
	rid += rsegBeg;

	return sol_segment.SegID[rid];
}
