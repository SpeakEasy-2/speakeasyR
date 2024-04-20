find_packages_i <- function(searchpath, pkgs) {
  libraries <- lapply(searchpath, dir)
  locs <- sapply(pkgs, function(p) {
    index <- 0
    for (i in seq_along(libraries)) {
      if (p %in% libraries[[i]]) {
        index <- i
        break
      }
    }

    if (!index) {
      stop(paste0("Error could not find package \"", p, "\" in library paths."))
    }

    file.path(searchpath[index], p, "include")
  })
}

deps <- desc::desc_get_deps()
pkgs <- deps$package[deps$type == "LinkingTo"]
cat(find_packages_i(.libPaths(), pkgs), sep = ";")
