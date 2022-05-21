// Set to 00:00:00:000 today
let today = new Date(),
  dateFormat = Highcharts.dateFormat;
// use millisecond as time stamp

// Set to 00:00:00:000 today
today.setUTCHours(0);
today.setUTCMinutes(0);
today.setUTCSeconds(0);
today.setUTCMilliseconds(0);
today = today.getTime();

// Create your data here
// rowData = [{
//   core: 1,
//   host: 1,
//   current: 0,
//   deals: [{
//     job: 1,
//     block: 1,
//     from: today + 0 * secondMultiplier,
//     to: today + 2 * secondMultiplier
//   }, {
//     job: 1,
//     block: 2,
//     from: today + 3 * secondMultiplier,
//     to: today + 5 * secondMultiplier
//   }
//   ]
// }, {
//   core: 2,
//   host: 1,
//   current: 0,
//   deals: [{
//     job: 2,
//     block: 1,
//     from: today + 2 * secondMultiplier,
//     to: today + 3 * secondMultiplier
//   }, {
//     job: 2,
//     block: 2,
//     from: today + 4 * secondMultiplier,
//     to: today + 7 * secondMultiplier
//   }
//   ]
// }
// ];

let jobToColor = {}

function getRandomColor() {
  let letters = '0123456789ABCDEF';
  let color = '#';
  for (let i = 0; i < 6; i++) {
    color += letters[Math.floor(Math.random() * 16)];
  }
  return color;
}

function getColorFromJob(jobId) {
  // Create job color if it doesn't exist
  if (!jobToColor.hasOwnProperty(jobId)) {
    jobToColor[jobId] = getRandomColor()
  }
  return jobToColor[jobId]
}

function createChart(seriesData) {
  // Parse car data into series.
  let series = seriesData.map(function (coreInfo, idx) {
    let data = coreInfo.deals.map(function (deal) {
      // A block in chart
      return {
        id: 'deal-' + idx,
        job: deal.job,
        block: deal.block,
        start: deal.from,
        end: deal.to,
        y: idx,
        color: getColorFromJob(deal.job)
      };
    });

    // Row info
    return {
      core: coreInfo.core,
      host: coreInfo.host,
      data: data,
      current: coreInfo.deals[coreInfo.deals.length - 1]
    };
  });

  Highcharts.ganttChart('container', {
    series: series,
    title: {
      text: 'Task Scheduling'
    },
    tooltip: {
      pointFormat: '<span>Job: {point.job}, Block: {point.block}</span><br/>' +
        '<span>From: {point.start:%M:%S.%L}</span><br/>' +
        '<span>To: {point.end:%M:%S.%L}</span>'
    },
    plotOptions: {
      series: {
        dataLabels: {
          enabled: true,
          format: '{point.job}',
          style: {
            cursor: 'default',
            pointerEvents: 'none'
          }
        }
      }
    },
    chart: {
      zoomType: "x"
    },
    xAxis: {
      // tickPixelInterval: 10000
    },
    yAxis: {
      type: 'category',
      grid: {
        columns: [{
          title: {
            text: 'Host'
          },
          categories: series.map(function (s) {
            return s.host;
          })
        }, {
          title: {
            text: 'Core'
          },
          categories: series.map(function (s) {
            return s.core;
            // return s.current.job;
          })
        }, {
          title: {
            text: 'Finish time'
          },
          categories: series.map(function (s) {
            return dateFormat('%M:%S.%L', s.current.to);
          })
        }]
      }
    },
    credits: {
      enabled: false
    }
  });
}

// File parse to json, call update
function loadData(data) {
  const jsonData = JSON.parse(data);
  let rowData = []
  
  Object.keys(jsonData).forEach(function(key) {
    rowData.push(jsonData[key])
  });

  createChart(rowData);
}

// File upload
document.getElementById('contentFile').onchange = function (evt) {
  try {
    let files = evt.target.files;
    if (!files.length) {
      alert('No file selected!');
      return;
    }
    
    let file = files[0];
    let reader = new FileReader();
    const self = this;
    
    reader.onload = (event) => {
      loadData(event.target.result)
    };
    
    reader.readAsText(file);
  } catch (err) {
    console.error(err);
  }
}