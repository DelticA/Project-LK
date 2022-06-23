function switchFilter(data, rootData, variables) {
  if (!data || !data.length) {
    return [];
  }
  var first = data[0];
  if (typeof first.value === "object") {
    var fields = ["at"].concat(Object.keys(first.value));
    return [fields].concat(
      data.map(function (d) {
        return [d.at].concat(Object.values(d.value));
      })
    );
  }
  return data.map(function (d) {
    return [Number(d.value)];
  });
}

function stateFilter(data, rootData, variables) {
  if (!data || !data.length) {
    return [];
  }
  var first = data[0];
  if (typeof first.value === "object") {
    var fields = ["at"].concat(Object.keys(first.value));
    return [fields].concat(
      data.map(function (d) {
        return [d.at].concat(Object.values(d.value));
      })
    );
  }
  return data.map(function (d) {
    return [d.value];
  });
}

function tempFilter(data, rootData, variables) {
  if (!data || !data.length) {
    return [];
  }
  var first = data[0];
  if (typeof first.value === "object") {
    var fields = ["at"].concat(Object.keys(first.value));
    return [fields].concat(
      data.map(function (d) {
        return [d.at].concat(Object.values(d.value));
      })
    );
  }
  return data.map(function (d) {
    return [(d.value / 100 - 6).toFixed(2), "ÎÂ¶È"];
  });
}

function smokeFilter(data, rootData, variables) {
  if (!data || !data.length) {
    return [];
  }
  var first = data[0];
  if (typeof first.value === "object") {
    var fields = ["at"].concat(Object.keys(first.value));
    return [fields].concat(
      data.map(function (d) {
        return [d.at].concat(Object.values(d.value));
      })
    );
  }
  return data.map(function (d) {
    return [(d.value / 10).toFixed(2), "ÑÌÎíÁ¿"];
  });
}

function textFilter(data, rootData, variables) {
  return [
    {
      value: data,
    },
  ];
}
