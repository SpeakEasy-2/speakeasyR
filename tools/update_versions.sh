ROOT=$(
    cd $(dirname $0)
    cd ..
    pwd
)
SE2="${ROOT}/tools/se2"
IGRAPH="${SE2}/vendor/igraph"

# Cmake usually gets version from git but git directory is not available at
# build time, need to create the version files to prevent cmake from failing.
cd "$IGRAPH"
git describe --tags >${ROOT}/IGRAPH_VERSION

cd "$SE2"
git describe --tags >${ROOT}/SE2_VERSION
