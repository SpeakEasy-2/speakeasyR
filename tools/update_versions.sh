TOOLS=$(
    cd "$(dirname "$0")"
    pwd
)
SE2="${TOOLS}/se2"
IGRAPH="${SE2}/vendor/igraph"

# Cmake usually gets version from git but git directory is not available at
# build time, need to create the version files to prevent cmake from failing.
cd "$IGRAPH" || exit
git describe --tags >"${TOOLS}/IGRAPH_VERSION"

cd "$SE2" || exit
git describe --tags >"${TOOLS}/SE2_VERSION"
