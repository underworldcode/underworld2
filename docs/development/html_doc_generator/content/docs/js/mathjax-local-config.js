/* Set local configuration for mathjax */

MathJax.Hub.Config({
    extensions: ["tex2jax.js"],
    jax: ["input/TeX", "output/HTML-CSS"],
    tex2jax: {
      inlineMath: [ ['$','$'], ["\\(","\\)"] ],
      displayMath: [ ['$$','$$'], ["\\[","\\]"] ],
      processEscapes: true
    },

    TeX: {
      equationNumbers: { autoNumber: "AMS" }
    },
    "HTML-CSS": {
      availableFonts: ["TeX"],
      scale: 85
    }
  });
