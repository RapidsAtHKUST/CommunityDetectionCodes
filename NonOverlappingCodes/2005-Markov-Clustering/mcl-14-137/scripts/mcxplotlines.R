
#  Example invocation:
#  R --vanilla --slave --args --exporder=R1 --exporder2=R2 --data=genes.tab --rowdict=genes.dict --coldict=region.dict --cls=G1 --pdf=out.pdf --clmin=5 --cl2min=15 < ~/mybin/mcxplotlines.R
#
#  cl2min is implicitly encoded in clustering, obtained by clm order.

library(R.utils)
library(gplots)

args <- R.utils::commandArgs(asValues=TRUE)

pdffile  <- "cls.pdf"
nmax     <- 10
fnexporder <- NULL
fnexporder2 <- NULL
randorder<- FALSE
collect_medians <- TRUE
minsubsize <- 2
minsupsize <- 5

rawfile  <- NULL
clsfile  <- NULL
fnrowdict  <- NULL
nlabel_max <- 100

if (!is.null(args[["nmax"]])) {
   nmax <- as.integer(args$nmax)
}
if (!is.null(args[["nlabel"]])) {
   nlabel_max <- as.integer(args$nlabel_max)
}
if (!is.null(args[["pdf"]])) {
   pdffile <- args$pdf
}
if (!is.null(args[["clmin"]])) {
   minsubsize <- as.integer(args$clmin)
}
if (!is.null(args[["cl2min"]])) {
   minsupsize <- as.integer(args$cl2min)
}
if (!is.null(args[["data"]])) {
   rawfile <- args$data
}
if (!is.null(args[["cls"]])) {
   clsfile <- args$cls
}
if (!is.null(args[["randorder"]])) {
   randorder <- TRUE
}
if (!is.null(args[["rowdict"]])) {
   fnrowdict <- args$rowdict
}
if (!is.null(args[["coldict"]])) {
   fncoldict <- args$coldict
}
if (!is.null(args[["exporder"]])) {
   fnexporder <- args$exporder
}
if (!is.null(args[["exporder2"]])) {
   fnexporder2 <- args$exporder2
}


if (is.null(rawfile) | is.null(clsfile) | is.null(fnrowdict)) {
   stop("need --data=<rawfile> --cls=<clsfile>} --tab=<fnrowdict> options")
}

exporder <- NULL
exporder_sizes <- NULL
exporder2_sizes <- NULL

if (!is.null(fnexporder)) {
   if (is.null(fncoldict)) {
      stop("with exporder I need a (column) dictionary file")
   }
   exporder <- scan(pipe(paste("mcxdump -icl", fnexporder, "-tabr", fncoldict)), what="character")
   exporder_tmp <- scan(pipe(paste("mcxdump -icl", fnexporder, "-tabr", fncoldict)), what="character", sep="\n")
   exporder_sizes <- unlist(lapply(exporder_tmp, function(x) { return(length(unlist(strsplit(x, "\t")))) }))
   if (!is.null(fnexporder2)) {
      exporder_tmp <- scan(pipe(paste("mcxdump -icl", fnexporder2, "-tabr", fncoldict)), what="character", sep="\n")
      exporder2_sizes <- unlist(lapply(exporder_tmp, function(x) { return(length(unlist(strsplit(x, "\t")))) }))
   }
}


mytbl <- read.table(rawfile, header=TRUE, row.names=1, check.names=F)
themin <- floor(min(mytbl))
themax <- floor(max(mytbl))

if (!is.null(exporder)) {
   if (length(exporder) != ncol(mytbl)) {
      stop(paste("experiment counts do not agree,", length(exporder), "vs", ncol(mytbl)))
   } else {
      mytbl <- mytbl[, exporder]
   }
} else if (randorder) {
   mytbl <- mytbl[, sample(1:ncol(mytbl), ncol(mytbl))]
}

cat("read rawfile\n")

command <- paste("mcxdump --no-values --transpose -imx",
                     clsfile, "-tabr", fnrowdict, "| sort -nk 2")
cat(paste("running", "[", command, "]\n"))
fobj <- pipe(command, "r")

cat("reading table ...\n")
cls <- read.table(fobj, header=FALSE, row.names=1,
         colClasses = c('character', 'factor'), check.names=TRUE)
close(fobj)
cat("done reading table\n")

clusters <- tapply(rownames(cls), cls$V2, function(x) { x })

mymin <- min(mytbl)
mymax <- max(mytbl)

# stopifnot(FALSE)


pdf(file=pdffile, height=8.26389,  width=11.6944)
par(mfrow=c(2,1), mai=c(2,0.5,0.5,0.5), cex=1.0, family='serif', lheight='1.2')


makegrid <- function() {
   if (!is.null(exporder_sizes)) {
      abline(v=cumsum(exporder_sizes), col="grey")
   }
   if (!is.null(exporder2_sizes)) {
      abline(v=cumsum(exporder2_sizes), col="black")
   }
}

# comedians <- matrix(ncol=0,nrow=ncol(mytbl))
# supercluster <- c()
# prevlen <- 10000000

summary_init <- function(names) {
   comedians <<- matrix(ncol=0,nrow=ncol(mytbl))
   supercluster <<- names
   prevlen <<- 10000000
}

summary_exec <- function() {
   super <- mytbl[supercluster,]
   l <- length(supercluster)
   matplot(
      comedians,
      type="n",
      main=paste("Collected subcluster medians for supercluster of size", l),
      fin=c(11,4),
      xaxs="i",
      ylim=c(themin,themax), pch=NA
   )
   makegrid()
   matlines(comedians, lty=1, col=colorpanel(ncol(comedians),low="yellow",high="darkorange"), pch=NA)
   lines(1:ncol(mytbl), apply(mytbl[supercluster,], 2, median), col = "black", lwd = 2)
}



mywrap <- function(x, thewidth = 0) {
   retval <- c("")
   if (thewidth <= 0) {
      thewidth <- par("din")[1] - par("omi")[2] - par("omi")[4] - par("mai")[2] - par("mai")[4]
   }

   for (i in seq_along(x)) {
      tststring <- sprintf("%s%s%s", retval[length(retval)], ifelse(i>1, " ", ""), x[i])
      if (strwidth(tststring, unit="inches") <= thewidth) {
         retval[length(retval)] <- tststring
      } else {
         retval <- c(retval, x[i])
      }
   }
   return(retval)
}

quants <- apply(mytbl, 2, function(x) { return(quantile(x, probs=c(0.25,0.75))) })

n_done <- 0

summary_init(c())

# comedians <- matrix(ncol=0,nrow=ncol(mytbl))        # ha.
# supercluster <- c()
# prevlen <- 10000000

for (id in names(clusters[order(as.integer(names(clusters)))])) {

   thelen <- length(clusters[[id]])
   if (thelen < minsubsize) {
      next
   }

   print(paste("cls", id, "size", thelen))

   # if (n_done == nmax) {
   #    break
   # }

   n_done <- n_done + 1

   sub <- mytbl[clusters[[id]],,drop=FALSE]

   ###
   if (collect_medians) {
      if (thelen <= prevlen) {
         comedians <- cbind(apply(sub, 2, median), comedians)
         supercluster <- c(supercluster, rownames(sub))
         prevlen <- thelen
      }
      else {
         # stopifnot(FALSE)
         ## matplot(comedians, type="n", main=paste("Collected subcluster medians"), fin=c(11,4), xaxs="i", ylim=c(themin,themax), pch=NA)
         ## makegrid()
         ## matlines(comedians, lty=1, col=colorpanel(ncol(comedians),low="yellow",high="darkorange"), pch=NA)
         ## lines(1:nrow(comedians), apply(comedians, 1, median), col="black", lwd = 2)
         ## comedians <- matrix(ncol=0,nrow=ncol(mytbl))
print(paste("supercluster of size", length(supercluster)))
         summary_exec()
         if (length(supercluster) < minsupsize) {
            break
         }
         summary_init(rownames(sub))
      }
   }
   ###

   matplot(t(sub), type="n", main=paste("Cluster ",id,", ", nrow(sub), " elements", sep=""), fin=c(11,4), xaxs="i", ylim=c(themin,themax), pch=NA)
   makegrid()

   matlines(t(sub), col="orange", lty=1, pch=NA)
   lines(1:ncol(sub), apply(sub, 2, median), col = "black", lwd = 2)

   thelabels <- sub("_.*", "", rownames(sub))
   if (length(thelabels) > nlabel_max) {
      thelabels <- c(sprintf("Label samples (%d/%d):", nlabel_max, nrow(sub)),  sample(thelabels, nlabel_max))
   } else {
      thelabels <- c("Labels:", thelabels)
   }
   txt <- paste(thelabels, collapse=" ")
   line_ofs <- 2
                                          # thewidth <- round(par("din")[1]/par("cin")[1])
   parg <- mywrap(thelabels, 0)
   for (i in seq_along(parg)) {
      mtext(parg[[i]], side=1, outer=FALSE, adj=0, line_ofs)
      line_ofs <- line_ofs + 1
   }

   thehi <- c()
   thelo <- c()

   for (exp in colnames(sub)) {
      m <- median(sub[,exp])
      if (m >= quants[2,exp]) {
         thehi <- c(thehi, exp)
      }
      if (m <= quants[1,exp]) {
         thelo <- c(thelo, exp)
      }
   }

   if (length(thehi) > 10) { thehi <- sample(thehi, 10) }
   if (length(thelo) > 10) { thelo <- sample(thelo, 10) }

   parg <- mywrap(c("High:", thehi))
   line_ofs <- line_ofs + 1
   for (i in seq_along(parg)) {
      mtext(parg[[i]], side=1, outer=FALSE, adj=0, line=line_ofs)
      line_ofs <- line_ofs + 1
   }

   parg <- mywrap(c("Low:", thelo))
   line_ofs <- line_ofs + 1
   for (i in seq_along(parg)) {
      mtext(parg[[i]], side=1, outer=FALSE, adj=0, line=line_ofs)
      line_ofs <- line_ofs + 1
   }

# print(paste(rownames(sub)))

   # axis(2)
}

if (collect_medians & length(supercluster) >= minsupsize) {
   summary_exec()
}

dev.off()
cat(paste("Output is in", pdffile, "\n"))

