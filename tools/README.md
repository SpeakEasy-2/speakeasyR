# Managing external libraries

Since cmake will fail if it can't find the versions for either `speakeasy2` or `igraph` and it usually determines the version based on the git tag, which is not available at build time since `.git` is left behind, whenever the submodules are updated, manually create version files and commit to the top-level project with:

``` shell
# Update submodule.
# Assuming at the root of the project.
tools/update_versions.sh
# Commit with submodule update.
```
