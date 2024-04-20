/* eslint-disable react/no-unstable-nested-components */
import { DefaultNode, Graph } from "@visx/network"
import PropTypes from 'prop-types';
import { Zoom } from '@visx/zoom';
import { RectClipPath } from '@visx/clip-path';
import { scaleLinear } from '@visx/scale';


import { useState } from 'react';

const bg = '#212121';

const initialTransform = {
  scaleX: 0.5,
  scaleY: 0.5,
  translateX: 0,
  translateY: 0,
  skewX: 0,
  skewY: 0,
};


NetworkGraph.propTypes= {
  nodes_s:  PropTypes.array,
  links_s:  PropTypes.array,
  width: PropTypes.number, 
  height: PropTypes.number
}

function findMinMax(nodes, property) {
  if(nodes.length > 0){
    return [Math.min(...nodes.map(item => item[property])), Math.max(...nodes.map(item => item[property]))]
  }
  return [0,0];
}

function NetworkGraph({width, height, nodes_s, links_s}) {

  if(height > 0){
    height = height - 40;
  }

  const [showMiniMap, setShowMiniMap] = useState(true);
  // create graph

  const xScale = scaleLinear({
        domain: findMinMax(nodes_s, 'x'),
        range: [0, width * 2],
      });
  const yScale = scaleLinear({
        domain: findMinMax(nodes_s, 'y'),
        range: [0,height * 2],
      });

  let nodes = nodes_s.map(node => ({
    x: xScale(node.x),
    y: yScale(node.y)
  }));

  let links = links_s.map(({ source, target }) => ({
    "source": {
      "x": xScale(source.x),
      "y": yScale(source.y)
    },
    "target": {
      "x": xScale(target.x),
      "y": yScale(target.y)
    }
  }));
  const graph = {
    nodes,
    links
  }

  return(
    <div className="rounded-3xl">
      <Zoom
        width={width}
        height={height}
        initialTransformMatrix={initialTransform}
      >
        {(zoom) => (
          <div className="relative">
            <svg 
              width={width} 
              height={height} 
              style={{ cursor: zoom.isDragging ? 'grabbing' : 'grab', touchAction: 'none' }} ref={zoom.containerRef} >
              <RectClipPath id="zoom-clip" width={width} height={height} />
              <rect 
                width={width} 
                height={height} 
                rx={24} fill={bg} />
              <g transform={zoom.toString()}>
                <Graph
                  graph={graph}
                  top={20}
                  left={20}
                  nodeComponent={DefaultNode }
                  fill={"#f56342"}
                  linkComponent={({ link: { source, target } }) => (
                    <line
                      x1={source.x}
                      y1={source.y}
                      x2={target.x}
                      y2={target.y}
                      strokeWidth={2}
                      stroke="#999"
                      strokeOpacity={0.6}
                    />
                  )}
                />
              </g>
              <rect
                width={width}
                height={height}
                rx={24}
                fill="transparent"
                onTouchStart={zoom.dragStart}
                onTouchMove={zoom.dragMove}
                onTouchEnd={zoom.dragEnd}
                onMouseDown={zoom.dragStart}
                onMouseMove={zoom.dragMove}
                onMouseUp={zoom.dragEnd}
                onMouseLeave={() => {
                  if (zoom.isDragging) zoom.dragEnd();
                }}
              />
              {showMiniMap && (
                <g
                  clipPath="url(#zoom-clip)"
                  transform={`
                    scale(0.25, 0.25)
                    translate(${width * 4 - width - 60}, ${height * 4 - height - 60})
                  `}
                >
                  <rect width={width} height={height} fill="#1a1a1a"/>
                  {
                    <Graph
                      graph={graph}
                      top={0}
                      left={0}
                      nodeComponent={({ node: { color } }) =>
                        color ? <DefaultNode fill={color} /> : <DefaultNode />
                      }
                      linkComponent={({ link: { source, target } }) => (
                        <line
                          x1={source.x}
                          y1={source.y}
                          x2={target.x}
                          y2={target.y}
                          strokeWidth={2}
                          stroke="#999"
                          strokeOpacity={0.6}
                        />
                      )}
                    />
                  }
                  <rect
                    width={width}
                    height={height}
                    fill="white"
                    fillOpacity={0.2}
                    stroke="white"
                    strokeWidth={4}
                    transform={zoom.toStringInvert()}
                  />
                </g>
              )}
            </svg>
            <div className="controls">
              <button
                type="button"
                className="btn btn-zoom"
                onClick={() => zoom.scale({ scaleX: 1.2, scaleY: 1.2 })}
              >
                +
              </button>
              <button
                type="button"
                className="btn btn-zoom btn-bottom"
                onClick={() => zoom.scale({ scaleX: 0.8, scaleY: 0.8 })}
              >
                -
              </button>
              <button type="button" className="btn btn-lg" onClick={zoom.reset}>
                Reset
              </button>
            </div>
            <div className="mini-map">
              <button
                type="button"
                className="btn btn-lg"
                onClick={() => setShowMiniMap(!showMiniMap)}
              >
                {showMiniMap ? 'Hide' : 'Show'} Mini Map
              </button>
            </div>
          </div>
      )}
  </Zoom>
  <style>{`
        .btn {
          margin: 0;
          text-align: center;
          border: none;
          background: #2f2f2f;
          color: #888;
          padding: 0 4px;
          border-top: 1px solid #0a0a0a;
        }
        .btn-lg {
          font-size: 12px;
          line-height: 1;
          padding: 4px;
        }
        .btn-zoom {
          width: 26px;
          font-size: 22px;
        }
        .btn-bottom {
          margin-bottom: 1rem;
        }
        .controls {
          position: absolute;
          top: 15px;
          right: 15px;
          display: flex;
          flex-direction: column;
          align-items: flex-end;
        }
        .mini-map {
          position: absolute;
          bottom: 25px;
          right: 15px;
          display: flex;
          flex-direction: column;
          align-items: flex-end;
        }
        .relative {
          position: relative;
        }
      `}</style>
  </div>
  );
}


export default NetworkGraph;